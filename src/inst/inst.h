#ifndef inst_header
#define inst_header

#include <stdint.h>
/**
 * @brief 汇编代码的指令类型
 *
 */
typedef enum INST_TYPE
{
    MOVRR,
    MOVRM,
    MOVMR,
    PUSH,
    POP,
    CALL,
    RET,
    ADDRR,
    INST_TYPE_LENGTH, // 用于结尾，
} inst_type_t;

/**
 * @brief 操作数格式
 * 表示不同的寻址模式
 * 下面的 REG1 表示的是基址寄存器
 * REG2 表示的是变址寄存器
 * 计算方式是：
 * Imm + R[r1] + R[r2] * S
 */
typedef enum operand_format
{
    IMM,
    REG,
    M_IMM,
    M_REG,
    M_IMM_REG1,
    M_REG1_REG2,
    M_IMM_REG1_REG2,
    M_REG2_S,
    M_IMM_REG2_S,
    M_REG1_REG2_S,
    M_IMM_REG1_REG2_S,
    NONE,

} operand_format_t;

/**
 * @brief 汇编指令包含指令类型，与指令操作数两部分
 * 这里是描述指令操作数的结构体
 *
 */
typedef struct operand
{
    operand_format_t operand_format;

    int64_t imm;
    int64_t scal;
    uint64_t reg_b;
    uint64_t reg_i;

} operand_t;

/**
 * @brief 因为我们需要人肉翻译汇编指令，
 * 所以使用这个结构来描述每个指令上面的内容

 */
#define CODE_LENGTH 100
typedef struct INST
{
    // 指令类型 mov 或者 add
    inst_type_t inst_type;
    // 源操作数：%rax/内存地址 等
    operand_t src;
    // 目标操作数： %rsi 等
    operand_t dst;

    // 汇编指令的字符串表示，用于debug
    char code[CODE_LENGTH];
} inst_t;

/**
 * @brief 定义指令处理函数，比如遇到了 mov 指令，就执行 mov 指令的处理函数
 *
 */
typedef void (*inst_type_handler)(uint64_t, uint64_t);

inst_type_handler inst_type_handler_table[INST_TYPE_LENGTH];

void init_inst_type_handler_tabl();
void run_inst_cycle();

void movrr_handler(uint64_t src, uint64_t dst);
void movrm_handler(uint64_t src, uint64_t dst);
void movmr_handler(uint64_t src, uint64_t dst);
void push_handler(uint64_t src, uint64_t dst);
void pop_handler(uint64_t src, uint64_t dst);
void ret_handler(uint64_t src, uint64_t dst);
void add_handler(uint64_t src, uint64_t dst);

#endif