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

    // TODO: compute dst Section Header Table and write into buffer

    // TODO: merge the symbol content from ELF src into dst sections
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