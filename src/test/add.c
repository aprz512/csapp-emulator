#include <stdio.h>
#include <assert.h>
#include "headers/mmu.h"
#include "headers/common.h"
#include "headers/memory.h"
#include "headers/cpu.h"

void print_register();
void print_stack();

#define ADD_INST_LENGTH 15

static void init_register()
{
    cpu_reg.rax = 0x1234;
    cpu_reg.rbx = 0x0;
    cpu_reg.rcx = 0x5555554006c0;
    cpu_reg.rdx = 0xabcd0000;
    cpu_reg.rsi = 0x7fffffffdd68;
    cpu_reg.rdi = 0x1;
    cpu_reg.rbp = 0x7fffffffdc80;
    cpu_reg.rsp = 0x7fffffffdc60;

    // 将 rip 初始化为 main 函数的第一条指令地址
    cpu_pc.rip = MAX_INSTRUCTION_CHAR * sizeof(char) * 11 + 0x00400000;
}

static void check_register()
{
    assert(cpu_reg.rax == 0xabcd1234);
    assert(cpu_reg.rbx == 0x0);
    assert(cpu_reg.rcx == 0x5555554006c0);
    assert(cpu_reg.rdx == 0x1234);
    assert(cpu_reg.rsi == 0xabcd0000);
    assert(cpu_reg.rdi == 0x1234);
    assert(cpu_reg.rbp == 0x7fffffffdc80);
    assert(cpu_reg.rsp == 0x7fffffffdc60);
}

static void init_memory()
{
    // 我们只关心 rbp - rsp 的内存的值

    write64bits_dram(va2pa(0x7fffffffdc60), 0x00005555554006c0);
    write64bits_dram(va2pa(0x7fffffffdc68), 0x1234);
    write64bits_dram(va2pa(0x7fffffffdc70), 0xabcd0000);
    write64bits_dram(va2pa(0x7fffffffdc78), 0x0);
    write64bits_dram(va2pa(0x7fffffffdc80), 0x00005555554006c0);
}

static void check_memory()
{
    assert(read64bits_dram(va2pa(0x7fffffffdc60)) == 0x00005555554006c0);
    assert(read64bits_dram(va2pa(0x7fffffffdc68)) == 0x1234);
    assert(read64bits_dram(va2pa(0x7fffffffdc70)) == 0xabcd0000);
    assert(read64bits_dram(va2pa(0x7fffffffdc78)) == 0xabcd1234);
    assert(read64bits_dram(va2pa(0x7fffffffdc80)) == 0x00005555554006c0);
}

void AddTestEntry()
{
    init_register();
    init_memory();

    // 假设 add 函数的地址在 0x00400000
    char assembly[ADD_INST_LENGTH][MAX_INSTRUCTION_CHAR] = {
        "push   %rbp",             // 0
        "mov    %rsp,%rbp",        // 1
        "mov    %rdi,-0x18(%rbp)", // 2
        "mov    %rsi,-0x20(%rbp)", // 3
        "mov    -0x18(%rbp),%rdx", // 4
        "mov    -0x20(%rbp),%rax", // 5
        "add    %rdx,%rax",        // 6
        "mov    %rax,-0x8(%rbp)",  // 7
        "mov    -0x8(%rbp),%rax",  // 8
        "pop    %rbp",             // 9
        "retq",                    // 10
        "mov    %rdx,%rsi",        // 11
        "mov    %rax,%rdi",        // 12
        "callq  0x00400000",       // 13
        "mov    %rax,-0x8(%rbp)",  // 14
    };

    // copy to physical memory
    // add function address in 0x00400000
    for (int i = 0; i < ADD_INST_LENGTH; ++i)
    {
        writeinst_dram(va2pa(i * 0x40 + 0x00400000), assembly[i]);
    }

    int time = 0;
    while (time < ADD_INST_LENGTH)
    {
        instruction_cycle();
        print_register();
        print_stack();
        time++;
    }

    check_register();
    check_memory();
}
