#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "headers/common.h"
#include "headers/linker.h"
#include "headers/log.h"

void parse_elf(char *filename, elf_t *elf);
void free_elf(elf_t *elf);

int main()
{

    elf_t elf;

    parse_elf("../linker/sum.elf.txt", &elf);



    free_elf(&elf);

    return 0;
}
