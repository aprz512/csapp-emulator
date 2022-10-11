#include <stdio.h>
#include "cpu/register.h"
#include <assert.h>
#include "memory/dram.h"

typedef enum enum_size
{
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    end
} enum_size_t;

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

    reg.rip = 0x55555540068c;
}

void check_register()
{
    assert(reg.rax == 0xabcd1234);
    assert(reg.rbx == 0x0);
    assert(reg.rcx == 0x5555554006c0);
    assert(reg.rbx == 0x1234);
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

int main()
{
    // enum_size_t t;
    printf("size = %d\n", end);

    return 0;
}