#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "headers/common.h"
#include "headers/linker.h"
#include "headers/log.h"

int read_elf(const char *filename, char *bufaddr)
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
            // bufaddr 是一个指针
            char *linebuf = bufaddr + line_counter * MAX_ELF_FILE_WIDTH * sizeof(char);

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
                linebuf[i] = line[i];
                i++;
            }
            linebuf[i] = '\0';
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
    assert(string2uint((char *)bufaddr) == line_counter);
    return line_counter;
}