#include <stdio.h>
#include <assert.h>
#include "memory/dram.h"
#include "inst/inst.h"
#include "disk/elf.h"
#include "cpu/mmu.h"
#include "cpu/register.h"
#include "tools/print.h"
#include "headers/common.h"

void init_register()
{
    reg.rax = 0x1234;
    reg.rbx = 0x0;
    reg.rcx = 0x5555554006c0;
    reg.rdx = 0xabcd0000;
    reg.rsi = 0x7fffffffdd68;
    reg.rdi = 0x1;
    reg.rbp = 0x7fffffffdc80;
    reg.rsp = 0x7fffffffdc60;

    // 将 rip 初始化为 main 函数的第一条指令地址
    reg.rip = 0x55555540068c;
}

void check_register()
{
    assert(reg.rax == 0xabcd1234);
    assert(reg.rbx == 0x0);
    assert(reg.rcx == 0x5555554006c0);
    assert(reg.rdx == 0x1234);
    assert(reg.rsi == 0xabcd0000);
    assert(reg.rdi == 0x1234);
    assert(reg.rbp == 0x7fffffffdc80);
    assert(reg.rsp == 0x7fffffffdc60);
    // rip 需要计算我们虚拟指令的大小，再算地址
    // assert(reg.rip == 0x55555540069b);
}

void init_memory()
{
    // 我们只关心 rbp - rsp 的内存的值

    write64bits_dram(va2pa(0x7fffffffdc60), 0x00005555554006c0);
    write64bits_dram(va2pa(0x7fffffffdc68), 0x1234);
    write64bits_dram(va2pa(0x7fffffffdc70), 0xabcd0000);
    write64bits_dram(va2pa(0x7fffffffdc78), 0x0);
    write64bits_dram(va2pa(0x7fffffffdc80), 0x00005555554006c0);
}

void check_memory()
{
    assert(read64bits_dram(va2pa(0x7fffffffdc60)) == 0x00005555554006c0);
    assert(read64bits_dram(va2pa(0x7fffffffdc68)) == 0x1234);
    assert(read64bits_dram(va2pa(0x7fffffffdc70)) == 0xabcd0000);
    assert(read64bits_dram(va2pa(0x7fffffffdc78)) == 0xabcd1234);
    assert(read64bits_dram(va2pa(0x7fffffffdc80)) == 0x00005555554006c0);
}

// int main()
// {

    // // 生成程序汇编指令
    // build_inst();
    // init_inst_type_handler_table();

    // init_register();
    // init_memory();

    // // 需要从 main 函数的第一个指令开始执行
    // reg.rip = (uint64_t)&program[11];
    // // 修正 CALL 指令的地址
    // program[13].src.imm = &program[0];

    // print_register();
    // print_stack();

    // // 执行 x 条指令
    // size_t x = 15;
    // for (size_t i = 0; i < x; i++)
    // {
    //     run_inst_cycle();
    // }

    // check_register();
    // check_memory();

    // printf("success!!!\n");

//     return 0;
// }