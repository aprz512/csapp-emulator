#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "headers/common.h"
#include "headers/linker.h"
#include "headers/log.h"


int main()
{

    elf_t elf;

    parse_elf("../linker/sum.elf.txt", &elf);



    free_elf(&elf);

    return 0;
}
