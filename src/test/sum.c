#include "headers/common.h"
#include "headers/cpu.h"
#include "assert.h"
#include "headers/memory.h"
#include "headers/mmu.h"
#include <stdio.h>

void print_register();
void print_stack();

#define SUM_INST_lENGTH 20

static void init_register()
{
    cpu_reg.rax = 0x555555400680;
    cpu_reg.rbx = 0x0;
    cpu_reg.rcx = 0x5555554006b0;
    cpu_reg.rdx = 0x7fffffffdc88;
    cpu_reg.rsi = 0x7fffffffdc78;
    cpu_reg.rdi = 0x1;
    cpu_reg.r8 = 0x7ffff7dced80;
    cpu_reg.r9 = 0x7ffff7dced80;
    cpu_reg.r10 = 0x1;
    cpu_reg.r11 = 0x0;
    cpu_reg.r12 = 0x555555400540;
    cpu_reg.r13 = 0x7fffffffdc70;
    cpu_reg.r14 = 0x0;
    cpu_reg.r15 = 0x0;
    cpu_reg.rbp = 0x7fffffffdb90;
    cpu_reg.rsp = 0x7fffffffdb90;

    // 将 rip 初始化为 main 函数的第一条指令地址
    cpu_pc.rip = MAX_INSTRUCTION_CHAR * sizeof(char) * 17 + 0x00400000;
}

static void check_register()
{
    assert(cpu_reg.rax == 0xf);
    assert(cpu_reg.rbx == 0x0);
    assert(cpu_reg.rcx == 0x5555554006b0);
    assert(cpu_reg.rdx == 0xa);
    assert(cpu_reg.rsi == 0xf);
    assert(cpu_reg.rdi == 0x1);
    assert(cpu_reg.rbp == 0x7fffffffdb90);
    assert(cpu_reg.rsp == 0x7fffffffdb90);
    assert(cpu_reg.r8 == 0x7ffff7dced80);
    assert(cpu_reg.r9 == 0x7ffff7dced80);
    assert(cpu_reg.r10 == 0x1);
    assert(cpu_reg.r11 == 0x0);
    assert(cpu_reg.r12 == 0x555555400540);
    assert(cpu_reg.r13 == 0x7fffffffdc70);
    assert(cpu_reg.r14 == 0x0);
    assert(cpu_reg.r15 == 0x0);
    assert(cpu_reg.rbp == 0x7fffffffdb90);
    assert(cpu_reg.rsp == 0x7fffffffdb90);
}

// static void init_memory()
// {
//     // 我们只关心 rbp - rsp 的内存的值

//     write64bits_dram(va2pa(0x7fffffffdc60), 0x00005555554006c0);
//     write64bits_dram(va2pa(0x7fffffffdc68), 0x1234);
//     write64bits_dram(va2pa(0x7fffffffdc70), 0xabcd0000);
//     write64bits_dram(va2pa(0x7fffffffdc78), 0x0);
//     write64bits_dram(va2pa(0x7fffffffdc80), 0x00005555554006c0);
// }

static void check_memory()
{
    assert(read64bits_dram(va2pa(0x7fffffffdaf8)) == 1);
    assert(read64bits_dram(va2pa(0x7fffffffdb18)) == 2);
    assert(read64bits_dram(va2pa(0x7fffffffdb38)) == 3);
    assert(read64bits_dram(va2pa(0x7fffffffdb58)) == 4);
    assert(read64bits_dram(va2pa(0x7fffffffdb78)) == 5);
}

void SumTestEntry()
{
    init_register();

    // 假设 sum 函数的地址在 0x00400000
    char assembly[SUM_INST_lENGTH][MAX_INSTRUCTION_CHAR] = {
        "push   %rbp",                      // 0
        "mov    %rsp,%rbp",                 // 1
        "sub    $0x10,%rsp",                // 2
        "mov    %rdi,-0x8(%rbp)",           // 3
        "cmpq   $0x1,-0x8(%rbp)",           // 4
        "jne    0x00400200",                // 5：jump to 8
        "mov    $0x1,%eax",                 // 6
        "jmp    0x004003c0",                // 7：jump to 15
        "mov    -0x8(%rbp),%rax",           // 8
        "sub    $0x1,%rax",                 // 9
        "mov    %rax,%rdi",                 // 10
        "callq  0x00400000",                // 11
        "mov    %rax,%rdx",                 // 12
        "mov    -0x8(%rbp),%rax",           // 13
        "add    %rdx,%rax",                 // 14
        "leaveq ",                          // 15
        "retq   ",                          // 16
        "mov    $0x5,%edi",                 // 17
        "callq  0x00400000",                // 18
        "mov    %rax,%rsi",                 // 19
    };

    // copy to physical memory
    // add function address in 0x00400000
    for (int i = 0; i < SUM_INST_lENGTH; ++i)
    {
        writeinst_dram(va2pa(i * 0x40 + 0x00400000), assembly[i]);
    }

    int time = 0;
    // 因该不会超过100次才对
    while (time < 100 && (cpu_pc.rip <= (SUM_INST_lENGTH-1) * 0x40 + 0x00400000))
    {
        instruction_cycle();
        print_register();
        print_stack();
        time++;
    }

    check_register();
    check_memory();

    printf("sum done!!!\n");
}
