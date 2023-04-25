#ifndef CPU_HEADER
#define CPU_HEADER

#include <stdint.h>
#include <stdlib.h>

/*======================================*/
/*      registers                       */
/*======================================*/

// struct of registers in each core
// resource accessible to the core itself only

typedef struct
{
    // return value
    union
    {
        uint64_t rax;
        uint32_t eax;
        uint16_t ax;
        struct
        {
            uint8_t al;
            uint8_t ah;
        };
    };

    // callee saved
    union
    {
        uint64_t rbx;
        uint32_t ebx;
        uint16_t bx;
        struct
        {
            uint8_t bl;
            uint8_t bh;
        };
    };

    // 4th argument
    union
    {
        uint64_t rcx;
        uint32_t ecx;
        uint16_t cx;
        struct
        {
            uint8_t cl;
            uint8_t ch;
        };
    };
    // 3th argument
    union
    {
        uint64_t rdx;
        uint32_t edx;
        uint16_t dx;
        struct
        {
            uint8_t dl;
            uint8_t dh;
        };
    };
    // 2nd argument
    union
    {
        uint64_t rsi;
        uint32_t esi;
        uint16_t si;
        struct
        {
            uint8_t sil;
            uint8_t sih;
        };
    };
    // 1st argument
    union
    {
        uint64_t rdi;
        uint32_t edi;
        uint16_t di;
        struct
        {
            uint8_t dil;
            uint8_t dih;
        };
    };

    // callee saved frame pointer
    union
    {
        uint64_t rbp;
        uint32_t ebp;
        uint16_t bp;
        struct
        {
            uint8_t bpl;
            uint8_t bph;
        };
    };
    // stack pointer
    union
    {
        uint64_t rsp;
        uint32_t esp;
        uint16_t sp;
        struct
        {
            uint8_t spl;
            uint8_t sph;
        };
    };

    // 5th argument
    union
    {
        uint64_t r8;
        uint32_t r8d;
        uint16_t r8w;
        uint8_t r8b;
    };
    // 6th argument
    union
    {
        uint64_t r9;
        uint32_t r9d;
        uint16_t r9w;
        uint8_t r9b;
    };

    // caller saved
    union
    {
        uint64_t r10;
        uint32_t r10d;
        uint16_t r10w;
        uint8_t r10b;
    };
    // caller saved
    union
    {
        uint64_t r11;
        uint32_t r11d;
        uint16_t r11w;
        uint8_t r11b;
    };

    // callee saved
    union
    {
        uint64_t r12;
        uint32_t r12d;
        uint16_t r12w;
        uint8_t r12b;
    };
    // callee saved
    union
    {
        uint64_t r13;
        uint32_t r13d;
        uint16_t r13w;
        uint8_t r13b;
    };
    // callee saved
    union
    {
        uint64_t r14;
        uint32_t r14d;
        uint16_t r14w;
        uint8_t r14b;
    };
    // callee saved
    union
    {
        uint64_t r15;
        uint32_t r15d;
        uint16_t r15w;
        uint8_t r15b;
    };
} cpu_reg_t;
cpu_reg_t cpu_reg;

/*======================================*/
/*      condition code flags            */
/*======================================*/

// condition code flags of most recent (latest) operation
// condition codes will only be set by the following integer arithmetic instructions

/* integer arithmetic instructions
    inc     increment 1
    dec     decrement 1
    neg     negate
    not     complement
    ----------------------------
    add     add
    sub     subtract
    imul    multiply
    xor     exclusive or
    or      or
    and     and
    ----------------------------
    sal     left shift
    shl     left shift (same as sal)
    sar     arithmetic right shift
    shr     logical right shift
*/

/* comparison and test instructions
    cmp     compare
    test    test
*/

// the 4 flags be a uint64_t in total
typedef union
{
    uint64_t __flags_value;
    struct
    {
        // carry flag: detect overflow for unsigned operations
        uint16_t CF;
        // zero flag: result is zero
        uint16_t ZF;
        // sign flag: result is negative: highest bit
        uint16_t SF;
        // overflow flag: detect overflow for signed operations
        uint16_t OF;
    };
} cpu_flags_t;
cpu_flags_t cpu_flags;

/*======================================*/
/*            crogram counter           */
/*======================================*/

// program counter or instruction pointer
typedef union
{
    uint64_t rip;
    uint32_t eip;
} cpu_pc_t;
cpu_pc_t cpu_pc;

/*
     CR1是未定义的控制寄存器，供将来的处理器使用。

     CR2是页故障线性地址寄存器，保存最后一次出现页故障的全32位线性地址。

     CR3是页目录基址寄存器，保存页目录表的物理地址，页目录表总是放在以4K字节为单位的存储器边界上，因此，它的地址的低12位总为0，不起作用，即使写上内容，也不会被理会。

     CR4在Pentium系列（包括486的后期版本）处理器中才实现，它处理的事务包括诸如何时启用虚拟8086模式等
*/
typedef struct
{
    uint64_t cr1;
    uint64_t cr2;
    uint64_t cr3;   // 本来应该储存的是物理地址，但是我们的模拟器只有16KB的内存，储存不下页表。所以就将页表使用 malloc 的方式放在模拟器程序进程的heap中
    uint64_t cr4;
} cpu_cr_t;
cpu_cr_t cpu_controls;

// move to common.h to be shared by linker
// #define MAX_INSTRUCTION_CHAR 64
#define NUM_INSTRTYPE 14

// CPU's instruction cycle: execution of instructions
void instruction_cycle();

#endif