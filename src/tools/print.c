#include "tools/print.h"
#include "cpu/register.h"
#include "memory/dram.h"
#include "cpu/mmu.h"
#include <stdio.h>

/**
 * @brief 仿照 gdb 的格式打印栈地址的值
 *
 */
void print_stack()
{
    // 打印从 rsp 到 rbp 地址的值，固定为 5 * 8 个字节

    // 将 physical_memory 解释为 uint64_t 的指针，就可以读一个 uint64_t 的值出来
    uint64_t *rsp = (uint64_t *)&physical_memory[va2pa(reg.rsp)];

    printf("=====================>>> memory <<<=====================\n");
    printf("0x%016lx : %16lx %16lx\n", reg.rsp - 0x20, (uint64_t) * (rsp - 3), (uint64_t) * (rsp - 4));
    printf("0x%016lx : %16lx %16lx\n", reg.rsp - 0x10, (uint64_t) * (rsp - 1), (uint64_t) * (rsp - 2));
    printf("0x%016lx : %16lx %16lx\n", reg.rsp + 0x00, (uint64_t) * (rsp + 0), (uint64_t) * (rsp + 1));
    printf("0x%016lx : %16lx %16lx\n", reg.rsp + 0x10, (uint64_t) * (rsp + 2), (uint64_t) * (rsp + 3));
    printf("0x%016lx : %16lx %16lx\n", reg.rsp + 0x20, (uint64_t) * (rsp + 4), (uint64_t) * (rsp + 5));
    printf("0x%016lx : %16lx %16lx\n", reg.rsp + 0x30, (uint64_t) * (rsp + 6), (uint64_t) * (rsp + 7));
    printf("\n");
}

/**
 * @brief 仿照 gdb 的格式打印寄存器的值
 *
 */
void print_register()
{
    printf("=====================>>> register <<<=====================\n");
    printf("rax = %16lx\nrbx = %16lx\nrcx = %16lx\nrdx = %16lx\n",
           reg.rax, reg.rbx, reg.rcx, reg.rdx);
    printf("rsi = %16lx\nrdi = %16lx\nrbp = %16lx\nrsp = %16lx\n",
           reg.rsi, reg.rdi, reg.rbp, reg.rsp);
    printf("rip = %16lx\n", reg.rip);
    printf("\n");
}
