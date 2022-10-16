#include "inst/inst.h"
#include "disk/elf.h"
#include <stdlib.h>
#include <stdio.h>
#include "cpu/register.h"
#include "tools/print.h"
#include "cpu/mmu.h"
#include "memory/dram.h"

/**
 * @brief 对于M_ 类型的操作符，返回虚拟地址
 *
 * @param operand
 * @return uint64_t
 */
static uint64_t decode_operand(operand_t operand)
{
    if (operand.operand_format == IMM)
    {
        // 将 int64_t 转成 uint64_t (因为有负数，所以这里直接将 int64_t 解释为 uint64_t，使用的时候记得转回来)
        // return *((uint64_t *)&operand.imm);
        return *((uint64_t *)&operand.imm);
    }
    else if (operand.operand_format == REG)
    {
        // 返回的是寄存器的地址
        return operand.reg_b;
    }
    else if (operand.operand_format == M_IMM)
    {
        return operand.imm;
    }
    else if (operand.operand_format == M_REG)
    {
        // M_REG 是取内存地址为 reg 的内存块的值
        return *((uint64_t *)operand.reg_b);
    }
    else if (operand.operand_format == M_IMM_REG1)
    {
        return operand.imm + *((uint64_t *)operand.reg_b);
    }
    else if (operand.operand_format == M_REG1_REG2)
    {
        return *((uint64_t *)operand.reg_b) + *((uint64_t *)operand.reg_i);
    }
    else if (operand.operand_format == M_IMM_REG1_REG2)
    {
        return operand.imm + *((uint64_t *)operand.reg_b) + *((uint64_t *)operand.reg_i);
    }
    else if (operand.operand_format == M_REG2_S)
    {
        return *((uint64_t *)operand.reg_i) * operand.scal;
    }
    else if (operand.operand_format == M_IMM_REG2_S)
    {
        return operand.imm + *((uint64_t *)operand.reg_i) * operand.scal;
    }
    else if (operand.operand_format == M_REG1_REG2_S)
    {
        return *((uint64_t *)operand.reg_b) + *((uint64_t *)operand.reg_i) * operand.scal;
    }
    else if (operand.operand_format == M_IMM_REG1_REG2_S)
    {
        return operand.imm + *((uint64_t *)operand.reg_b) + *((uint64_t *)operand.reg_i) * operand.scal;
    }
    else if (operand.operand_format == NONE)
    {
        // do nothing
    }
    else
    {
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
    inst_type_handler_table[MOVRR] = &movrr_handler;
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
    // 取指令地址
    inst_t *inst = (inst_t *)reg.rip;
    // printf("program i code = %s\n", inst->code);
    // printf("program i type = %d\n", inst->inst_type);
    // printf("program i src.operand_format = %d\n", inst->src.operand_format);
    // printf("program i dst.operand_format = %d\n", inst->dst.operand_format);
    // printf("program i src.imm = %x\n", inst->src.imm);
    // printf("program i dst.imm = %x\n", inst->dst.imm);

    // printf("program call code = %s\n", program[13].code);
    // printf("program call src.imm = %x\n", program[13].src.imm);

    // 输出调试信息
    printf("\n run inst code: %s\n", inst->code);

    // 译码
    uint64_t src = decode_operand(inst->src);
    uint64_t dst = decode_operand(inst->dst);

    // printf("func addr = %x\n", movrr_handler);
    // printf("table fun addr = %x\n", inst_type_handler_table[inst->inst_type]);

    // 执行
    inst_type_handler handler = inst_type_handler_table[inst->inst_type];
    handler(src, dst);

    print_register();
    print_stack();
}

/**
 * @brief mov %rax, %rbx
 *
 * @param src 寄存器的地址
 * @param dst 寄存器的地址
 */
void movrr_handler(uint64_t src, uint64_t dst)
{
    *(uint64_t *)dst = *(uint64_t *)src;
    reg.rip += sizeof(inst_t);
}

/**
 * @brief mov %rax, 0x8(%rbx)
 *
 * @param src 寄存器地址
 * @param dst 虚拟内存的地址
 */
void movrm_handler(uint64_t src, uint64_t dst)
{
    uint64_t value = *(uint64_t *)src;
    write64bits_dram(va2pa(dst), value);
    reg.rip += sizeof(inst_t);
}

/**
 * @brief mov 0x8(%rax), %rbx
 *
 * @param src 虚拟内存的地址
 * @param dst 寄存器的地址
 */
void movmr_handler(uint64_t src, uint64_t dst)
{
    uint64_t value = read64bits_dram(va2pa(src));
    *(uint64_t *)dst = value;
    reg.rip += sizeof(inst_t);
}

/**
 * @brief push   %rbp
 *
 * @param src 寄存器地址
 * @param dst 无
 */
void push_handler(uint64_t src, uint64_t dst)
{
    reg.rsp = reg.rsp - 0x8;
    uint64_t value = *(uint64_t *)src;
    write64bits_dram(va2pa(reg.rsp), value);
    reg.rip += sizeof(inst_t);
}

/**
 * @brief pop    %rbp
 *
 * @param src 寄存器地址
 * @param dst 无
 */
void pop_handler(uint64_t src, uint64_t dst)
{
    uint64_t value = read64bits_dram(va2pa(reg.rsp));
    *(uint64_t *)src = value;
    reg.rsp = reg.rsp + 0x8;
    reg.rip += sizeof(inst_t);
}

void ret_handler(uint64_t src, uint64_t dst)
{
    uint64_t value = read64bits_dram(va2pa(reg.rsp));
    reg.rsp = reg.rsp + 0x8;
    reg.rip = value;
}

/**
 * @brief add    %rdx,%rax
 *
 * @param src 寄存器地址
 * @param dst 寄存器地址
 */
void addrr_handler(uint64_t src, uint64_t dst)
{
    *(uint64_t *)dst = *(uint64_t *)src + *(uint64_t *)dst;
    reg.rip += sizeof(inst_t);
}

/**
 * @brief callq  0x555555400520 <printf@plt>
 * 将下一条指令的地址压入栈中
 *
 * @param src 地址
 * @param dst 无
 */
void call_handler(uint64_t src, uint64_t dst)
{
    reg.rsp = reg.rsp - 0x8;
    // next inst addr
    uint64_t value = reg.rip + sizeof(inst_t);
    write64bits_dram(va2pa(reg.rsp), value);

    reg.rip = src;
}