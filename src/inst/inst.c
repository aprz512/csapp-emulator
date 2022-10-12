#include "inst/inst.h"
#include "disk/elf.h"
#include <stdlib.h>
#include <stdio.h>
#include "cpu/register.h"
#include "tools/print.h"
#include "cpu/mmu.h"
#include "memory/dram.h"

/**
 * @brief 一定要返回地址，handler 才好处理
 *
 * @param operand
 * @return uint64_t
 */
static uint64_t decode_operand(operand_t operand)
{
    if (operand.operand_format == IMM)
    {
        return operand.imm;
    }
    else if (operand.operand_format == REG)
    {
        return operand.reg_b;
    }
    else if (operand.operand_format == M_IMM)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.imm)];
    }
    else if (operand.operand_format == M_REG)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.reg_b)];
    }
    else if (operand.operand_format == M_IMM_REG1)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.reg_b + operand.imm)];
    }
    else if (operand.operand_format == M_REG1_REG2)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.reg_b + operand.reg_i)];
    }
    else if (operand.operand_format == M_IMM_REG1_REG2)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.reg_b + operand.imm + operand.reg_i)];
    }
    else if (operand.operand_format == M_REG2_S)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.scal * operand.reg_i)];
    }
    else if (operand.operand_format == M_IMM_REG2_S)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.reg_i * operand.scal + operand.imm)];
    }
    else if (operand.operand_format == M_REG1_REG2_S)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.reg_b + operand.scal * operand.reg_i)];
    }
    else if (operand.operand_format == M_IMM_REG1_REG2_S)
    {
        // 返回物理内存的地址
        return &physical_memory[va2pa(operand.imm + operand.reg_b + operand.scal * operand.reg_i)];
    } else {
        printf("unknown operand format: %d\n", operand.operand_format);
        exit(-1);
    }
}

/**
 * @brief 初始化指令处理函数表
 *
 */
void init_inst_type_handler_table()
{
    inst_type_handler_table[MOVRR] = movrr_handler;
    inst_type_handler_table[MOVRM] = movrm_handler;
    inst_type_handler_table[MOVMR] = movmr_handler;
    inst_type_handler_table[PUSH] = push_handler;
    inst_type_handler_table[POP] = pop_handler;
    inst_type_handler_table[RET] = ret_handler;
    inst_type_handler_table[ADDRR] = addrr_handler;
    inst_type_handler_table[CALL] = call_handler;
}

void run_inst_cycle()
{
    // 取地址
    inst_t *inst = (inst_t *)reg.rip;

    // 译码
    uint64_t src = decode_operand(inst->src);
    uint64_t dst = decode_operand(inst->dst);

    // 执行
    inst_type_handler handler = inst_type_handler_table[inst->inst_type];
    handler(src, dst);

    // 输出调试信息
    printf("inst code: %s\n", inst->code);
    print_register();
    print_stack();
}

void movrr_handler(uint64_t src, uint64_t dst)
{
    *(uint64_t *)dst = *(uint64_t *)src;
    reg.rip += sizeof(inst_t);
}

void movrm_handler(uint64_t src, uint64_t dst) {}
void movmr_handler(uint64_t src, uint64_t dst) {}
void push_handler(uint64_t src, uint64_t dst) {}
void pop_handler(uint64_t src, uint64_t dst) {}
void ret_handler(uint64_t src, uint64_t dst) {}
void addrr_handler(uint64_t src, uint64_t dst) {}
void call_handler(uint64_t src, uint64_t dst) {}