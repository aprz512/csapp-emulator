#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "headers/common.h"
#include "headers/linker.h"
#include "headers/log.h"

void free_elf(elf_t *elf);
void parse_elf(char *filename, elf_t *elf);
static int read_elf(const char *filename, char (*elf_buf)[MAX_ELF_FILE_WIDTH]);
static int parse_table_entry(char *str, char ***ent);
static void print_elf(elf_t *elf);
static void print_section_header_entry(sh_entry_t *sh);
static void parse_section_header(char *str, sh_entry_t *sh);
static void free_table_entry(char **ent, int n);
static void parse_symtab(char *str, st_entry_t *ste);
static void print_symtab_entry(st_entry_t *ste);



/**
 * @brief 将 elf txt 文件读入 buf 中
 *
 * @param filename rootProject/linker/sum.elf.txt
 * @return elf 文件的有效长度
 */
static int read_elf(const char *filename, char (*elf_buf)[MAX_ELF_FILE_WIDTH])
{
    // open file and read
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        my_log(DEBUG_LINKER, "unable to open file %s\n", filename);
        exit(1);
    }

    // read text file line by line
    char line[MAX_ELF_FILE_WIDTH];
    int line_counter = 0;

    while (fgets(line, MAX_ELF_FILE_WIDTH, fp) != NULL)
    {
        int len = strlen(line);
        // 去换行与注释行
        if ((len == 0) ||
            (len >= 1 && (line[0] == '\n' || line[0] == '\r')) ||
            (len >= 2 && (line[0] == '/' && line[1] == '/')))
        {
            continue;
        }

        // 去除空白行
        int iswhite = 1;
        for (int i = 0; i < len; ++i)
        {
            iswhite = iswhite && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r');
        }
        if (iswhite == 1)
        {
            continue;
        }

        // 剩下的是有效行，需要限制行数
        if (line_counter < MAX_ELF_FILE_LENGTH)
        {
            int i = 0;
            // 限制 txt 文件一行的宽度
            while (i < len && i < MAX_ELF_FILE_WIDTH)
            {
                // 过滤掉换行符与行内注释内容
                if ((line[i] == '\n') ||
                    (line[i] == '\r') ||
                    ((i + 1 < len) && (i + 1 < MAX_ELF_FILE_WIDTH) && line[i] == '/' && line[i + 1] == '/'))
                {
                    break;
                }

                elf_buf[line_counter][i] = line[i];
                i++;
            }
            elf_buf[line_counter][i] = '\0';
            line_counter++;
        }
        else
        {
            my_log(DEBUG_LINKER, "elf file %s is too long (>%d)\n", filename, MAX_ELF_FILE_LENGTH);
            fclose(fp);
            exit(1);
        }
    }

    fclose(fp);
    assert(string2uint((char *)elf_buf) == line_counter);
    return line_counter;
}

/**
 * @brief 解析 elf 文件
 */
void parse_elf(char *filename, elf_t *elf)
{
    assert(elf != NULL);

    int line_count = read_elf(filename, elf->buffer);
    elf->line_count = line_count;
    elf->sht_count = string2uint(elf->buffer[1]);
    print_elf(elf);

    // parse section headers
    int sh_count = elf->sht_count;
    elf->sht = malloc(sh_count * sizeof(sh_entry_t));

    sh_entry_t *symt_sh = NULL;
    for (int i = 0; i < sh_count; ++i)
    {
        parse_section_header(elf->buffer[2 + i], &(elf->sht[i]));
        print_section_header_entry(&(elf->sht[i]));

        if (strcmp(elf->sht[i].sh_name, ".symtab") == 0)
        {
            // this is the section header for symbol table
            symt_sh = &(elf->sht[i]);
        }
    }

    assert(symt_sh != NULL);

    // 解析符号表
    elf->symt_count = symt_sh->sh_size;
    elf->symt = malloc(elf->symt_count * sizeof(st_entry_t));
    for (int i = 0; i < symt_sh->sh_size; ++ i)
    {
        parse_symtab(
            elf->buffer[i + symt_sh->sh_offset],
            &(elf->symt[i]));
        print_symtab_entry(&(elf->symt[i]));
    }
}

static void print_elf(elf_t *elf)
{
    for (int i = 0; i < elf->line_count; ++i)
    {
        my_log(DEBUG_LINKER, "[elf]-[%d]\t%s\n", i, elf->buffer[i]);
    }
}

static void print_section_header_entry(sh_entry_t *sh)
{
    my_log(DEBUG_LINKER, "[section-header-entry]\t%s\t%x\t%d\t%d\n",
           sh->sh_name,
           sh->sh_addr,
           sh->sh_offset,
           sh->sh_size);
}

void free_elf(elf_t *elf)
{
    assert(elf != NULL);

    free(elf->sht);
}

static void parse_section_header(char *str, sh_entry_t *sh)
{
    char **cols;
    int num_cols = parse_table_entry(str, &cols);
    assert(num_cols == 4);

    strcpy(sh->sh_name, cols[0]);
    sh->sh_addr = string2uint(cols[1]);
    sh->sh_offset = string2uint(cols[2]);
    sh->sh_size = string2uint(cols[3]);

    free_table_entry(cols, num_cols);
}

static void free_table_entry(char **ent, int n)
{
    for (int i = 0; i < n; ++i)
    {
        free(ent[i]);
    }
    free(ent);
}

static int parse_table_entry(char *str, char ***ent)
{
    // parse line as table entries
    int count_col = 1;
    int len = strlen(str);

    // count columns
    for (int i = 0; i < len; ++i)
    {
        if (str[i] == ',')
        {
            count_col++;
        }
    }

    // malloc and create list
    char **arr = malloc(count_col * sizeof(char *));
    *ent = arr;

    int col_index = 0;
    int col_width = 0;
    char col_buf[32];
    for (int i = 0; i < len + 1; ++i)
    {
        if (str[i] == ',' || str[i] == '\0')
        {
            assert(col_index < count_col);

            // malloc and copy
            char *col = malloc((col_width + 1) * sizeof(char));
            for (int j = 0; j < col_width; ++j)
            {
                col[j] = col_buf[j];
            }
            col[col_width] = '\0';

            // update
            arr[col_index] = col;
            col_index++;
            col_width = 0;
        }
        else
        {
            assert(col_width < 32);
            col_buf[col_width] = str[i];
            col_width++;
        }
    }

    return count_col;
}

static void parse_symtab(char *str, st_entry_t *ste)
{
    // sum,STB_GLOBAL,STT_FUNC,.text,0,22
    char **cols;
    int num_cols = parse_table_entry(str, &cols);
    assert(num_cols == 6);

    assert(ste != NULL);
    strcpy(ste->st_name, cols[0]);

    // select symbol bind
    if (strcmp(cols[1], "STB_LOCAL") == 0)
    {
        ste->bind = STB_LOCAL;
    }
    else if (strcmp(cols[1], "STB_GLOBAL") == 0)
    {
        ste->bind = STB_GLOBAL;
    }
    else if (strcmp(cols[1], "STB_WEAK") == 0)
    {
        ste->bind = STB_WEAK;
    }
    else
    {
        printf("symbol bind is neiter LOCAL, GLOBAL, nor WEAK\n");
        exit(0);
    }
    
    // select symbol type 
    if (strcmp(cols[2], "STT_NOTYPE") == 0)
    {
        ste->type = STT_NOTYPE;
    }
    else if (strcmp(cols[2], "STT_OBJECT") == 0)
    {
        ste->type = STT_OBJECT;
    }
    else if (strcmp(cols[2], "STT_FUNC") == 0)
    {
        ste->type = STT_FUNC;
    }
    else
    {
        printf("symbol type is neiter NOTYPE, OBJECT, nor FUNC\n");
        exit(0);
    }

    strcpy(ste->st_shndx, cols[3]);

    ste->st_value = string2uint(cols[4]);
    ste->st_size = string2uint(cols[5]);

    free_table_entry(cols, num_cols);
}

static void print_symtab_entry(st_entry_t *ste)
{
    my_log(DEBUG_LINKER, "[symbol-table]\t%s\t%d\t%d\t%s\t%d\t%d\n",
        ste->st_name,
        ste->bind,
        ste->type,
        ste->st_shndx,
        ste->st_value,
        ste->st_size);
}
