#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <headers/linker.h>
#include <headers/common.h>
#include <headers/log.h>

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst, smap_t *map, int *smap_count);
static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate);
static void print_smap_table(smap_t *smap_table, int smap_count);
static inline int symbol_precedence(st_entry_t *sym);
static void compute_section_header(elf_t *dst, smap_t *smap_table, int *smap_count);
static void merge_section(elf_t **srcs, int num_srcs, elf_t *dst, smap_t *smap_table, int *smap_count);
static void print_output_elf(elf_t *dst);
static const char *get_stt_string(st_type_t type);
static const char *get_stb_string(st_bind_t bind);

/* ------------------------------------ */
/* Exposed Interface for Static Linking */
/* ------------------------------------ */
void link_elf(elf_t **srcs, int num_srcs, elf_t *dst)
{
    // reset the destination since it's a new file
    memset(dst, 0, sizeof(elf_t));

    // create the map table to connect the source elf files and destination elf file symbols
    int smap_count = 0;
    smap_t smap_table[MAX_SYMBOL_MAP_LENGTH];

    // update the smap table - symbol processing
    symbol_processing(srcs, num_srcs, dst, (smap_t *)&smap_table, &smap_count);

    print_smap_table(smap_table, smap_count);

    compute_section_header(dst, smap_table, &smap_count);
    dst->symt_count = smap_count;
    dst->symt = malloc(dst->sht_count * sizeof(st_entry_t));
    merge_section(srcs, num_srcs, dst, smap_table, &smap_count);

    print_output_elf(dst);
}

static void print_smap_table(smap_t *smap_table, int smap_count)
{
    for (int i = 0; i < smap_count; ++i)
    {
        st_entry_t *ste = smap_table[i].src;
        my_log(DEBUG_LINKER, "[smap]\t%s\t%d\t%d\t%s\t%d\t%d\n",
               ste->st_name,
               ste->bind,
               ste->type,
               ste->st_shndx,
               ste->st_value,
               ste->st_size);
    }
}

// 这里是静态链接，不处理 dst
static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst,
                              smap_t *smap_table, int *smap_count)
{
    // for every elf files
    for (int i = 0; i < num_srcs; ++i)
    {
        elf_t *elfp = srcs[i];

        // for every symbol from this elf file
        for (int j = 0; j < elfp->symt_count; ++j)
        {
            st_entry_t *sym = &(elfp->symt[j]);

            if (sym->bind == STB_LOCAL)
            {
                // 能够通过编译，说明每个 elf 里面的 local 名字不会冲突
                // 每一个 LOCAL 符号都放到 dst 里面，因为不同源文件的 local 符号不冲突
                assert(*smap_count < MAX_SYMBOL_MAP_LENGTH);
                smap_table[*smap_count].src = sym;
                smap_table[*smap_count].elf = elfp;
            }
            else if (sym->bind == STB_GLOBAL)
            {
                // global 符号需要遍历 map 中已有的符号，如果优先级更强，则替换
                for (int k = 0; k < *smap_count; ++k)
                {
                    st_entry_t *candidate = smap_table[k].src;
                    if (candidate->bind == STB_GLOBAL &&
                        (strcmp(candidate->st_name, sym->st_name) == 0))
                    {
                        // 有名字冲突，需要选择优先级高的
                        simple_resolution(sym, elfp, &smap_table[k]);
                        goto NEXT_SYMBOL_PROCESS;
                    }
                }
                // 无冲突，直接存进去
                assert(*smap_count <= MAX_SYMBOL_MAP_LENGTH);
                // update map table
                smap_table[*smap_count].src = sym;
                smap_table[*smap_count].elf = elfp;
                (*smap_count)++;
            }

        NEXT_SYMBOL_PROCESS:
            // do nothing
            ;
        }
    }

    // 遍历 map，检查看是否还有没有定义的符号
    // 链接完成之后，不应该还有未定义的符号
    for (int i = 0; i < *smap_count; ++i)
    {
        st_entry_t *s = smap_table[i].src;

        assert(strcmp(s->st_shndx, "SHN_UNDEF") != 0);
        assert(s->type != STT_NOTYPE);

        // 检查 common，这里只是声明了符号，没有定义
        // 遍历完成之后，如果外部有定义了强符号的，应该被替换掉了，还没有就是默认值，去 .bss
        if (strcmp(s->st_shndx, "COMMON") == 0)
        {
            char *bss = ".bss";
            for (int j = 0; j < MAX_CHAR_SECTION_NAME; ++j)
            {
                if (j < 4)
                {
                    s->st_shndx[j] = bss[j];
                }
                else
                {
                    s->st_shndx[j] = '\0';
                }
            }
            s->st_value = 0;
        }
    }
}

static inline int symbol_precedence(st_entry_t *sym)
{

    /*
        暂不讨论weak符号，写起来太麻烦了
            bind        type        shndx               prec
            --------------------------------------------------
            global      notype      undef               0 - undefined
            global      object      common              1 - tentative
            global      object      data,bss,rodata     2 - defined
            global      func        text                2 - defined

            weak        func        text                1 - weak defined
            weak        object      data,bss,rodata     1 - weak defined
    */
    // 未定义的符号，优先级为 0
    if (strcmp(sym->st_shndx, "SHN_UNDEF") == 0 && sym->type == STT_NOTYPE)
    {
        return 0;
    }

    if (strcmp(sym->st_shndx, "COMMON") == 0 && sym->type == STT_OBJECT)
    {
        // 这个时候，符号应该放到哪个section，需要根据初始值来判断
        return 1;
    }

    if ((strcmp(sym->st_shndx, ".text") == 0 && sym->type == STT_FUNC) ||
        (strcmp(sym->st_shndx, ".data") == 0 && sym->type == STT_OBJECT) ||
        (strcmp(sym->st_shndx, ".rodata") == 0 && sym->type == STT_OBJECT) ||
        (strcmp(sym->st_shndx, ".bss") == 0 && sym->type == STT_OBJECT))
    {
        // Defined
        return 2;
    }

    my_log(DEBUG_LINKER, "[symbol resolution]: 暂时不知道该符号 \"%s\" 的优先级", sym->st_name);
    exit(0);
}

static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate)
{
    // src: symbol from current elf file
    // dst: pointer to the internal map table slot: src -> dst

    // determines which symbol is the one to be kept with 3 rules
    // rule 1: multiple strong symbols with the same name are not allowed
    // rule 2: given a strong symbol and multiple weak symbols with the same name, choose the strong symbol
    // rule 3: given multiple weak symbols with the same name, choose any of the weak symbols
    int src_pre = symbol_precedence(sym);
    int candidate_pre = symbol_precedence(candidate->src);
    if (src_pre == 2 && candidate_pre == 2)
    {
        my_log(DEBUG_LINKER,
               "[symbol resolution]: 发现了重复定义的强符号 \"%s\" 声明\n", sym->st_name);
        exit(0);
    }
    else
    {
        // 高优先级替换低优先级
        if (src_pre > candidate_pre)
        {
            candidate->src = sym;
            candidate->elf = sym_elf;
        }
    }
}

static void compute_section_header(elf_t *dst, smap_t *smap_table, int *smap_count)
{
    // 我们的例子中只有3中section .text .rodata .data
    // .bss 不占用 section，因为里面全是 0
    int count_text = 0, count_rodata = 0, count_data = 0;
    for (int i = 0; i < *smap_count; ++i)
    {
        st_entry_t *sym = smap_table[i].src;

        if (strcmp(sym->st_shndx, ".text") == 0)
        {
            // .text section symbol
            count_text += sym->st_size;
        }
        else if (strcmp(sym->st_shndx, ".rodata") == 0)
        {
            // .rodata section symbol
            count_rodata += sym->st_size;
        }
        else if (strcmp(sym->st_shndx, ".data") == 0)
        {
            // .data section symbol
            count_data += sym->st_size;
        }
        else
        {
            my_log(DEBUG_LINKER, "[linker]\t暂不支持的section %s\n", sym->st_shndx);
            exit(1);
        }
    }

    // 计算 section header 个数
    dst->sht_count = (count_text != 0) + (count_rodata != 0) + (count_data != 0) + 1;
    // 总行数： 1 + 1 + section header count + section count * secton num
    dst->line_count = 1 + 1 + dst->sht_count + count_text + count_rodata + count_data + *smap_count;
    // the target dst: line_count, sht_count, sht, .text, .rodata, .data, .symtab
    // print to buffer
    sprintf(dst->buffer[0], "%ld", dst->line_count);
    sprintf(dst->buffer[1], "%ld", dst->sht_count);

    // x86-64 linux 系统中，代码段总是从 0x400000 开始
    uint64_t text_runtime_addr = 0x400000;
    uint64_t rodata_runtime_addr = text_runtime_addr + count_text * MAX_INSTRUCTION_CHAR * sizeof(char);
    uint64_t data_runtime_addr = rodata_runtime_addr + count_rodata * sizeof(uint64_t);
    // 符号表不加载
    uint64_t symtab_runtime_addr = 0;

    // write the section header table
    assert(dst->sht == NULL);
    dst->sht = malloc(dst->sht_count * sizeof(sh_entry_t));

    // write in .text, .rodata, .data order
    uint64_t section_offset = 1 + 1 + dst->sht_count;
    int sh_index = 0;
    sh_entry_t *sh = NULL;
    if (count_text > 0)
    {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);

        // write the fields
        strcpy(sh->sh_name, ".text");
        sh->sh_addr = text_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_text;

        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
               sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        // update the index
        sh_index++;
        section_offset += sh->sh_size;
    }
    if (count_rodata > 0)
    {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);

        // write the fields
        strcpy(sh->sh_name, ".rodata");
        sh->sh_addr = rodata_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_rodata;

        // write to buffer
        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
                sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        // update the index
        sh_index++;
        section_offset += sh->sh_size;
    }

    if (count_data > 0)
    {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);

        // write the fields
        strcpy(sh->sh_name, ".data");
        sh->sh_addr = data_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_data;

        // write to buffer
        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
                sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        // update the index
        sh_index++;
        section_offset += sh->sh_size;
    }

    // .symtab
    assert(sh_index < dst->sht_count);
    sh = &(dst->sht[sh_index]);

    // write the fields
    strcpy(sh->sh_name, ".symtab");
    sh->sh_addr = symtab_runtime_addr;
    sh->sh_offset = section_offset;
    sh->sh_size = *smap_count;

    // write to buffer
    sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

    assert(sh_index + 1 == dst->sht_count);
}

static void print_output_elf(elf_t *dst)
{
    my_log(DEBUG_LINKER, "[output]\t-------------------------------------\n");
    my_log(DEBUG_LINKER, "[output]\tDestination ELF's SHT in Buffer:\n");
    for (int i = 0; i < dst->line_count; ++i)
    {
        my_log(DEBUG_LINKER, "[output]\t%s\n", dst->buffer[i]);
    }
}

// precondition: dst should know the section offset of each section
// merge the target section lines from ELF files and update dst symtab
static void merge_section(elf_t **srcs, int num_srcs, elf_t *dst,
                          smap_t *smap_table, int *smap_count)
{
    int line_written = 1 + 1 + dst->sht_count;
    int symt_written = 0;
    int sym_section_offset = 0;

    for (int section_index = 0; section_index < dst->sht_count; ++section_index)
    {
        // get the section by section id
        sh_entry_t *target_sh = &dst->sht[section_index];
        sym_section_offset = 0;

        // 遍历每个 elf 文件，找到对应的 section
        for (int i = 0; i < num_srcs; ++i)
        {
            int src_section_index = -1;
            // scan every section in this elf
            for (int j = 0; j < srcs[i]->sht_count; ++j)
            {
                if (strcmp(target_sh->sh_name, srcs[i]->sht[j].sh_name) == 0)
                {
                    // we have found the same section name
                    src_section_index = j;
                    // break;
                }
            }

            // 没找到 section
            if (src_section_index == -1)
            {
                // search for the next ELF
                continue;
            }
            else
            {
                // 找到 section 了，遍历其符号表
                for (int j = 0; j < srcs[i]->symt_count; ++j)
                {
                    st_entry_t *sym = &srcs[i]->symt[j];
                    // 是匹配的 section
                    if (strcmp(sym->st_shndx, target_sh->sh_name) == 0)
                    {
                        // 遍历 map 中的section
                        for (int k = 0; k < *smap_count; ++k)
                        {
                            // 这个 sym 在map中，所以该 section 需要合入
                            if (sym == smap_table[k].src)
                            {
                                for (int t = 0; t < sym->st_size; ++t)
                                {
                                    int dst_index = line_written + t;
                                    int src_index = srcs[i]->sht[src_section_index].sh_offset +
                                                    sym->st_value + t;

                                    assert(dst_index < MAX_ELF_FILE_LENGTH);
                                    assert(src_index < MAX_ELF_FILE_LENGTH);

                                    strcpy(
                                        dst->buffer[dst_index],
                                        srcs[i]->buffer[src_index]);
                                }
                                assert(symt_written < dst->symt_count);
                                // copy the entry
                                strcpy(dst->symt[symt_written].st_name, sym->st_name);
                                dst->symt[symt_written].bind = sym->bind;
                                dst->symt[symt_written].type = sym->type;
                                strcpy(dst->symt[symt_written].st_shndx, sym->st_shndx);
                                // MUST NOT BE A COMMON, so the section offset MUST NOT BE alignment
                                dst->symt[symt_written].st_value = sym_section_offset;
                                dst->symt[symt_written].st_size = sym->st_size;

                                // update the smap_table
                                // this will hep the relocation
                                smap_table[k].dst = &dst->symt[symt_written];

                                // udpate the counter
                                symt_written += 1;
                                line_written += sym->st_size;
                                sym_section_offset += sym->st_size;
                            }
                        }
                    }
                    // symbol srcs[i].symt[j] has been checked
                }
                // ELF file srcs[i] has been checked
            }
        }
        // dst.sht[section_index] has been merged from src ELFs
    }
    // all sections in dst has been merged
    for (int i = 0; i < dst->symt_count; ++i)
    {
        st_entry_t *sym = &dst->symt[i];
        sprintf(dst->buffer[line_written], "%s,%s,%s,%s,%ld,%ld",
                sym->st_name, get_stb_string(sym->bind), get_stt_string(sym->type),
                sym->st_shndx, sym->st_value, sym->st_size);
        line_written++;
    }
    assert(line_written == dst->line_count);
}

static const char *get_stb_string(st_bind_t bind)
{
    switch (bind)
    {
        case STB_GLOBAL:
            return "STB_GLOBAL";
        case STB_LOCAL:
            return "STB_LOCAL";
        case STB_WEAK:
            return "STB_WEAK";
        default:
            printf("incorrect symbol bind\n");
            exit(0);
    }
}

static const char *get_stt_string(st_type_t type)
{
    switch (type)
    {
        case STT_NOTYPE:
            return "STT_NOTYPE";
        case STT_OBJECT:
            return "STT_OBJECT";
        case STT_FUNC:
            return "STT_FUNC";
        default:
            printf("incorrect symbol type\n");
            exit(0);
    }
}