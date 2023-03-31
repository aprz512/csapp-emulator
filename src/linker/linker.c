#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "headers/common.h"
#include "headers/linker.h"
#include "headers/common.h"

int main()
{
    int elf_num = 2;
    char elf_fn[2][64] = {
        "main",
        "sum"};
    elf_t **srcs = tag_malloc(elf_num * sizeof(elf_t *), "link");
    for (int i = 0; i < elf_num; ++i)
    {
        char elf_fullpath[100];
        sprintf(elf_fullpath, "%s/%s.elf.txt", "../linker", elf_fn[i]);
        printf("%s\n", elf_fullpath);

        srcs[i] = tag_malloc(sizeof(elf_t), "link");
        parse_elf(elf_fullpath, srcs[i]);
    }

    elf_t linked;
    link_elf(srcs, elf_num, &linked);

    char eof_fullpath[100];
    sprintf(eof_fullpath, "%s/%s.eof.txt", "../linker", "output");
    printf("into %s\n", eof_fullpath);

    write_eof(eof_fullpath, &linked);

    // releaes elf heap
    for (int i = 0; i < elf_num; ++i)
    {
        free_elf(srcs[i]);
    }

    tag_free(srcs);

    return 0;
}
