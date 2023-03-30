#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "headers/common.h"
#include "headers/linker.h"
#include "headers/log.h"

int parse_section_header_entry(char *str, char ***ent);
int read_elf(const char *filename, char (*elf_buf)[MAX_ELF_FILE_WIDTH]);
void parse_elf(char *filename, elf_t *elf);
void print_elf(elf_t *elf);
void print_section_header_entry(sh_entry_t *sh);
void free_elf(elf_t *elf);
void parse_section_header(char *str, sh_entry_t *sh);
void free_section_header_entry(char **ent, int n);


/**
 * @brief 将 elf txt 文件读入 buf 中
 *
 * @param filename rootProject/linker/sum.elf.txt
 * @return elf 文件的有效长度
 */
int read_elf(const char *filename, char (*elf_buf)[MAX_ELF_FILE_WIDTH])
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
    int line_count = read_elf(filename, elf->buffer);
    elf->line_count = line_count;
    elf->sht_count = string2uint(elf->buffer[1]);
    print_elf(elf);

    // parse section headers
    int sh_count = elf->sht_count;
    elf->sht = malloc(sh_count * sizeof(sh_entry_t));
    for (int i = 0; i < sh_count; ++i)
    {
        parse_section_header(elf->buffer[2 + i], &(elf->sht[i]));
        print_section_header_entry(&(elf->sht[i]));
    }
}

void print_elf(elf_t *elf)
{
    for (int i = 0; i < elf->line_count; ++i)
    {
        my_log(DEBUG_LINKER, "[elf]-[%d]\t%s\n", i, elf->buffer[i]);
    }
}

void print_section_header_entry(sh_entry_t *sh)
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

void parse_section_header(char *str, sh_entry_t *sh)
{
    char **cols;
    int num_cols = parse_section_header_entry(str, &cols);
    assert(num_cols == 4);

    strcpy(sh->sh_name, cols[0]);
    sh->sh_addr = string2uint(cols[1]);
    sh->sh_offset = string2uint(cols[2]);
    sh->sh_size = string2uint(cols[3]);

    free_section_header_entry(cols, num_cols);
}

void free_section_header_entry(char **ent, int n)
{
    for (int i = 0; i < n; ++i)
    {
        free(ent[i]);
    }
    free(ent);
}

/**
 * @brief 解析 section header entry
 *  .text,0x0,4,22
 *  .symtab,0x0,26,2
 */
int parse_section_header_entry(char *str, char ***ent)
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
