#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "headers/common.h"
#include "headers/linker.h"
#include "headers/log.h"

int read_elf(const char *filename, char (*elf_buf)[MAX_ELF_FILE_WIDTH]);

int main()
{

    char elf[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];

    printf("%c \n", elf[3][2]);

    int count = read_elf("../linker/sum.elf.txt", elf);
    for (int i = 0; i < count; ++i)
    {
        printf("%s\n", elf[i]);
    }

    // char *buffer = getcwd(NULL, 0);
    // printf("%s\n",buffer);

    return 0;
}
