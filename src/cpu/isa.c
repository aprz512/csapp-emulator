#include <stdint.h>
#include <string.h>
#include "headers/log.h"
#include "headers/common.h"

/*======================================*/
/*      instruction set architecture    */
/*======================================*/

// data structures
// 定义指令的操作符
typedef enum INST_OPERATOR
{
    INST_MOV,   // 0
    INST_PUSH,  // 1
    INST_POP,   // 2
    INST_LEAVE, // 3
    INST_CALL,  // 4
    INST_RET,   // 5
    INST_ADD,   // 6
    INST_SUB,   // 7
    INST_CMP,   // 8
    INST_JNE,   // 9
    INST_JMP,   // 10
} op_t;

/**
 * @brief 指令操作数格式
 * 表示不同的寻址模式
 * 下面的 REG1 表示的是基址寄存器
 * REG2 表示的是变址寄存器
 * 计算方式是：
 * Imm + R[r1] + R[r2] * S
 */
typedef enum OPERAND_TYPE
{
    EMPTY,                 // 0
    IMM,                   // 1
    REG,                   // 2
    MEM_IMM,               // 3
    MEM_REG1,              // 4
    MEM_IMM_REG1,          // 5
    MEM_REG1_REG2,         // 6
    MEM_IMM_REG1_REG2,     // 7
    MEM_REG2_SCAL,         // 8
    MEM_IMM_REG2_SCAL,     // 9
    MEM_REG1_REG2_SCAL,    // 10
    MEM_IMM_REG1_REG2_SCAL // 11
} od_type_t;

typedef struct OPERAND_STRUCT
{
    od_type_t type; // IMM, REG, MEM
    uint64_t imm;   // immediate number
    uint64_t scal;  // scale number to register 2
    uint64_t reg1;  // main register
    uint64_t reg2;  // register 2
} od_t;

// local variables are allocated in stack in run-time
// we don't consider local STATIC variables
// ref: Computer Systems: A Programmer's Perspective 3rd
// Chapter 7 Linking: 7.5 Symbols and Symbol Tables
typedef struct INST_STRUCT
{
    op_t op;  // enum of operators. e.g. mov, call, etc.
    od_t src; // operand src of instruction
    od_t dst; // operand dst of instruction
} inst_t;

/*======================================*/
/*      parse assembly instruction      */
/*======================================*/

static void parse_instruction(const char *str, inst_t *inst)
{
    size_t str_len = strlen(str);

    char op[64] = {'\0'};
    size_t op_len = 0;

    char od1[64] = {'\0'};
    size_t od1_len = 0;

    char od2[64] = {'\0'};
    size_t od2_len = 0;

    char cur = 0;
    int comma_count = 0;
    int brackets_count = 0;
    int state = 0;
    for (size_t i = 0; i < str_len; i++)
    {
        cur = str[i];

        // 数括号和逗号
        if (cur == '(' || cur == ')')
        {
            brackets_count++;
        }
        else if (cur == ',')
        {
            if (brackets_count == 0 || brackets_count == 2)
            {
                comma_count++;
            }
        }

        // 状态变化
        if (state == 0)
        {
            if (cur != ' ')
            {
                state = 1;
            }
        }
        else if (state == 1)
        {
            if (cur == ' ')
            {
                state = 2;
            }
        }
        else if (state == 2)
        {
            if (cur != ' ')
            {
                state = 3;
            }
        }
        else if (state == 3)
        {
            // 解析 od1，以逗号为标准
            if (comma_count == 1)
            {
                state = 4;
            }
        }
        else if (state == 4)
        {
            if (cur != ',')
            {
                state = 5;
            }
        }
        else if (state == 5)
        {
            if (cur == ' ')
            {
                state = 6;
            }
        }

        // 计算 op od1 od2
        if (state == 1)
        {
            op[op_len++] = cur;
        }
        else if (state == 3)
        {
            od1[od1_len++] = cur;
        }
        else if (state == 5)
        {
            od2[od2_len++] = cur;
        }
    }

    my_log(DEBUG_PARSEINST, "[%s] [%s] [%s]\n", op, od1, od2);
    // log(DEBUG_PARSEINST, "[%s (%d)] [%s (%d)] [%s (%d)]\n", op, inst->op, od1, inst->src.type, od2, inst->dst.type);
}

void TestParseInstruction() {
        char assembly[15][MAX_INSTRUCTION_CHAR] = {
        "push   %rbp",              // 0
        "mov    %rsp,%rbp",         // 1
        "mov    %rdi,-0x18(%rbp)",  // 2
        "mov    %rsi,-0x20(%rbp)",  // 3
        "mov    -0x18(%rbp,%rax,8),%rdx",  // 4
        "mov    -0x20(%rbp),%rax",  // 5
        "add    %rdx,%rax",         // 6
        "mov    %rax,-0x8(%rbp)",   // 7
        "mov    -0x8(%rbp),%rax",   // 8
        "pop    %rbp",              // 9
        "retq",                     // 10
        "mov    %rdx,%rsi",         // 11
        "mov    %rax,%rdi",         // 12
        "callq  0",                 // 13
        "mov    %rax,-0x8(%rbp)",   // 14
    };
    
    inst_t inst;
    for (int i = 0; i < 15; ++ i)
    {
        parse_instruction(assembly[i], &inst);
    }
}