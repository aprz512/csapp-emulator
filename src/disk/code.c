#include "inst/inst.h"
#include "disk/elf.h"
#include "cpu/register.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


void build_inst()
{

    inst_t *result = malloc(sizeof(inst_t) * INST_LEN);
    if (result == NULL)
    {
        printf("malloc failed.\n");
        return;
    }

    inst_t temp[INST_LEN] = {

        // 这里不能传寄存器的值，需要传递寄存器的地址才行
        // add 汇编片段
        {
            PUSH,
            {REG, 0, 0, &reg.rbp, 0},
            {NONE, 0, 0, 0, 0},
            "push   %rbp",
        },
        {
            MOVRR,
            {REG, 0, 0, &reg.rsp, 0},
            {REG, 0, 0, &reg.rbp, 0},
            "mov    %rsp,%rbp",
        },
        {
            MOVRM,
            {REG, 0, 0, &reg.rdi, 0},
            {M_IMM_REG1, -0x18, 0, &reg.rbp, 0},
            "mov    %rdi,-0x18(%rbp)",
        },
        {
            MOVRM,
            {REG, 0, 0, &reg.rsi, 0},
            {M_IMM_REG1, -0x20, 0, &reg.rbp, 0},
            "mov    %rsi,-0x20(%rbp)",
        },

        {
            MOVMR,
            {M_IMM_REG1, -0x18, 0, &reg.rbp, 0},
            {REG, 0, 0, &reg.rdx, 0},
            "mov    -0x18(%rbp),%rdx",
        },

        {
            MOVMR,
            {M_IMM_REG1, -0x20, 0, &reg.rbp, 0},
            {REG, 0, 0, &reg.rax, 0},
            "mov    -0x20(%rbp),%rax",
        },

        {
            ADDRR,
            {REG, 0, 0, &reg.rdx, 0},
            {REG, 0, 0, &reg.rax, 0},
            "add    %rdx,%rax",
        },
        {
            MOVRM,
            {REG, 0, 0, &reg.rax, 0},
            {M_IMM_REG1, -0x8, 0, &reg.rbp, 0},
            "mov    %rax,-0x8(%rbp)",
        },
        {
            MOVMR,
            {M_IMM_REG1, -0x8, 0, &reg.rbp, 0},
            {REG, 0, 0, &reg.rax, 0},
            "mov    -0x8(%rbp),%rax",
        },
        {
            POP,
            {REG, 0, 0, &reg.rbp, 0},
            {NONE, 0, 0, 0, 0},
            "pop    %rbp",
        },
        {
            RET,
            {NONE, 0, 0, 0, 0},
            {NONE, 0, 0, 0, 0},
            "retq",
        },

        // main 汇编片段
        {
            MOVRR,
            {REG, 0, 0, &reg.rdx, 0},
            {REG, 0, 0, &reg.rsi, 0},
            "mov    %rdx,%rsi",
        },
        {
            MOVRR,
            {REG, 0, 0, &reg.rax, 0},
            {REG, 0, 0, &reg.rdi, 0},
            "mov    %rax,%rdi",
        },
        {
            CALL,
            // 这里的地址要填 add 函数的第一个指令的地址，也就是数组的第一个元素的地址
            {IMM, 0, 0, 0, 0},
            {NONE, 0, 0, 0, 0},
            "callq  0x55555540064a <add>",
        },
        {
            MOVRM,
            {REG, 0, 0, &reg.rax, 0},
            {M_IMM_REG1, -0x8, 0, &reg.rbp, 0},
            "mov    %rax,-0x8(%rbp)",
        },
    };

    for (size_t i = 0; i < INST_LEN; i++)
    {
        *(result + i) = *(temp + i);
    }

    for (size_t i = 0; i < INST_LEN; i++)
    {
        program[i] = temp[i];
    }

}