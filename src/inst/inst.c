#include "inst/inst.h"
#include "disk/elf.h"
#include <stdlib.h>
#include "cpu/register.h"

static uint64_t decode_operand(operand_t operand) {}

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
}

void movrr_handler(uint64_t src, uint64_t dst) {
    *(uint64_t*)dst = *(uint64_t*) src;
    reg.rip += sizeof(inst_t);
}

void movrm_handler(uint64_t src, uint64_t dst){}
void movmr_handler(uint64_t src, uint64_t dst){}
void push_handler(uint64_t src, uint64_t dst){}
void pop_handler(uint64_t src, uint64_t dst){}
void ret_handler(uint64_t src, uint64_t dst){}
void addrr_handler(uint64_t src, uint64_t dst){}
void call_handler(uint64_t src, uint64_t dst){}