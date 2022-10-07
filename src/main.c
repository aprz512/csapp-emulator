#include <stdio.h>
#include <cpu/register.h>
#include <assert.h>

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
    reg.rsi = 0x7fffffffe078;
    reg.rdi = 0x1;
    reg.rbp = 0x7fffffffdf90;
    reg.rsp = 0x7fffffffdf70;
}

void check_register()
{
    assert(reg.rax == 0xabcd1234);
    assert(reg.rbx == 0x0);
    assert(reg.rcx == 0x5555554006c0);
    assert(reg.rbx == 0x1234);
    assert(reg.rsi == 0xabcd0000);
    assert(reg.rdi == 0x1234);
    assert(reg.rbp == 0x7fffffffdf90);
    assert(reg.rsp == 0x7fffffffdf70);
}

void init_memory() {
    // 我们只关心 rsp 附近的内存的值
}

void check_memory() {

}

int main()
{
    // enum_size_t t;
    printf("size = %d\n", end);

    return 0;
}