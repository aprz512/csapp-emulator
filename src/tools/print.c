#include "tools/print.h"
#include "cpu/register.h"
#include "memory/dram.h"


/**
 * @brief 仿照 gdb 的格式打印栈地址的值
 * 
 */
void print_stack(){
    int n = 10;

    // TODO

    // uint64_t *high = (uint64_t*)&physical_memory[va2pa(reg.rsp)];
    // high = &high[n];

    // uint64_t rsp_start = reg.rsp + n * 8;

    // for (int i = 0; i < 2 * n; ++ i)
    // {
    //     uint64_t *ptr = (uint64_t *)(high - i);
    //     printf("0x%016lx : %16lx", rsp_start, (uint64_t)*ptr);

    //     if (i == n)
    //     {
    //         printf(" <== rsp");
    //     }

    //     rsp_start = rsp_start - 8;

    //     printf("\n");
    // }
}

/**
 * @brief 仿照 gdb 的格式打印寄存器的值
 * 
 */
void print_register(){
    printf("rax = %16lx\nrbx = %16lx\nrcx = %16lx\nrdx = %16lx\n",
        reg.rax, reg.rbx, reg.rcx, reg.rdx);
    printf("rsi = %16lx\nrdi = %16lx\nrbp = %16lx\nrsp = %16lx\n",
        reg.rsi, reg.rdi, reg.rbp, reg.rsp);
    printf("rip = %16lx\n", reg.rip);
}
