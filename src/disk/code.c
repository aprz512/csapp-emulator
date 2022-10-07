#include <inst/inst.h>
#include <disk/elf.h>
#include <cpu/register.h>
#include <stdint.h>
#include <malloc.h>
#include <stdio.h>


inst_t *build_code()
{

    inst_t *result = malloc(sizeof(inst_t) * INST_LEN);
    if (result == NULL)
    {
        printf("malloc failed.\n");
        return NULL;
    }

    inst_t program[INST_LEN] = {

        // add 汇编片段
        {
            PUSH,
            {REG, 0, 0, reg.rbp, 0},
            {NONE, 0, 0, 0, 0}},
        {
            MOVRR,
            {REG, 0, 0, reg.rsp, 0},
            {REG, 0, 0, reg.rbp, 0},
        },
        {
            MOVRR,
            {REG, 0, 0, reg.rdi, 0},
            {REG, -0x8, 0, reg.rbp, 0},
        },
        {
            MOVRR,
            {REG, 0, 0, reg.rsi, 0},
            {REG, -0x10, 0, reg.rbp, 0},
        },

        {
            MOVRR,
            {REG, -0x8, 0, reg.rbp, 0},
            {REG, 0, 0, reg.rdx, 0},
        },

        {
            MOVRR,
            {REG, -0x10, 0, reg.rbp, 0},
            {REG, 0, 0, reg.rax, 0},
        },

        {
            ADDRR,
            {REG, 0, 0, reg.rdx, 0},
            {REG, 0, 0, reg.rax, 0},
        },
        {
            POP,
            {REG, 0, 0, reg.rbp, 0},
            {NONE, 0, 0, 0, 0},
        },

        // main 汇编片段
        {
            MOVRR,
            {REG, 0, 0, reg.rdx, 0},
            {REG, 0, 0, reg.rsi, 0},
        },
        {
            MOVRR,
            {REG, 0, 0, reg.rax, 0},
            {REG, 0, 0, reg.rdi, 0},
        },
        {
            CALL,
            // 这里的地址要填 add 函数的第一个指令的地址，也就是数组的第一个元素的地址
            {NONE, 0, 0, 0, 0},
            {NONE, 0, 0, 0, 0},
        },
        {
            MOVRR,
            {REG, 0, 0, reg.rax, 0},
            {REG, -0x8, 0, reg.rbp, 0},
        },
    };

    for (size_t i = 0; i < INST_LEN; i++)
    {
        *(result + i) = *(program + i);
    }

    // 修正 result 里面的 CALL 指令的地址
    inst_t call_inst = *(result + 11);
    call_inst.src.imm = result;

    return program;
}