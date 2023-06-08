#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "headers/common.h"
#include "headers/common.h"
#include "headers/cpu.h"
#include "headers/mytrie.h"
#include "headers/memory.h"
#include "headers/mmu.h"
#include "headers/instruction.h"
#include "headers/interrupt.h"
#include "headers/color.h"

/*======================================*/
/*      parse assembly instruction      */
/*======================================*/

// functions to map the string assembly code to inst_t instance
static uint64_t parse_register(const char *str);
static void parse_instruction(const char *str, inst_t *inst);
static void parse_operand(const char *str, od_t *od);
static uint64_t compute_operand(od_t *od);
static int parse_operator(const char *str);

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
        //
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
        // else if (state == 5)
        // {
        //     if (cur == ' ')
        //     {
        //         state = 6;
        //     }
        // }

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

    inst->op = parse_operator(op);

    if (od1_len > 0)
    {
        parse_operand(od1, &inst->src);
    }

    if (od2_len > 0)
    {
        parse_operand(od2, &inst->dst);
    }

    my_log(DEBUG_PARSEINST, "[%s (%d)] [%s (%d, %x, %x, %x, %d)] [%s (%d, %x, %x, %x, %d)]\n",
           op, inst->op,
           od1, inst->src.type, inst->src.imm, inst->src.reg1, inst->src.reg2, inst->src.scal,
           od2, inst->dst.type, inst->dst.imm, inst->dst.reg1, inst->dst.reg2, inst->dst.scal);
}

static void parse_operand(const char *str, od_t *od)
{
    // clear od struct
    od->type = -1;
    od->imm = 0;
    od->reg1 = 0;
    od->reg2 = 0;
    od->scal = 0;

    if (str[0] == '$')
    {
        od->type = IMM;
        od->imm = string2uint_range(str, 1, -1);
    }
    else if (str[0] == '%')
    {
        od->type = REG;
        od->reg1 = parse_register(str);
    }
    else
    {
        char cur = 0;
        int comma_count = 0;
        int brackets_count = 0;
        int len = strlen(str);

        char imm[64] = {'\0'};
        int imm_len = 0;

        char reg1[64] = {'\0'};
        int reg1_len = 0;

        char reg2[64] = {'\0'};
        int reg2_len = 0;

        char scale[64] = {'\0'};
        int scale_len = 0;

        for (size_t i = 0; i < len; i++)
        {
            cur = str[i];
            if (cur == '(' || cur == ')')
            {
                brackets_count++;
                // we do not need brackets
                continue;
            }
            else if (cur == ',')
            {
                comma_count++;
                // we do not need comma
                continue;
            }

            if (brackets_count == 0 && comma_count == 0)
            {
                // imm(
                imm[imm_len++] = cur;
            }
            else if (brackets_count == 1 && comma_count == 0)
            {
                // imm(reg1,
                reg1[reg1_len++] = cur;
            }
            else if (brackets_count == 1 && comma_count == 1)
            {
                // imm(reg1,reg2,
                reg2[reg2_len++] = cur;
            }
            else if (brackets_count == 1 && comma_count == 2)
            {
                // imm(reg1,reg2,scale
                scale[scale_len++] = cur;
            }
            else
            {
                // done
            }
        }

        // set od value
        if (imm_len > 0)
        {
            od->imm = string2uint(imm);
        }

        if (reg1_len > 0)
        {
            od->reg1 = parse_register(reg1);
        }

        if (reg2_len > 0)
        {
            od->reg2 = parse_register(reg2);
        }

        if (scale_len > 0)
        {
            od->scal = string2uint(scale);
            if (od->scal != 1 && od->scal != 2 && od->scal != 4 && od->scal != 8)
            {
                my_log(DEBUG_PARSEINST, "scale value illegal： %s\n", scale);
                exit(0);
            }
        }

        // set od type
        if (brackets_count == 0)
        {
            od->type = MEM_IMM;
        }
        else if (brackets_count == 2 && comma_count == 0)
        {
            if (imm_len == 0)
            {
                od->type = MEM_REG1;
            }
            else
            {
                od->type = MEM_IMM_REG1;
            }
        }
        else if (brackets_count == 2 && comma_count == 1)
        {
            if (imm_len == 0)
            {
                od->type = MEM_REG1_REG2;
            }
            else
            {
                od->type = MEM_IMM_REG1_REG2;
            }
        }
        else if (brackets_count == 2 && comma_count == 2)
        {
            if (reg1_len > 0)
            {
                if (imm_len == 0)
                {
                    od->type = MEM_REG1_REG2_SCAL;
                }
                else
                {
                    od->type = MEM_IMM_REG1_REG2_SCAL;
                }
            }
            else
            {
                if (imm_len == 0)
                {
                    od->type = MEM_REG2_SCAL;
                }
                else
                {
                    od->type = MEM_IMM_REG2_SCAL;
                }
            }
        }
    }

    my_log(DEBUG_PARSEINST, "operand : %s, parse result : [%d][%lx][%lx][%lx][%d]\n", str, od->type, od->imm, od->reg1, od->reg2, od->scal);
}

// interpret the operand
static uint64_t compute_operand(od_t *od)
{
    if (od->type == IMM)
    {
        // immediate signed number can be negative: convert to bitmap
        return *(uint64_t *)&od->imm;
    }
    else if (od->type == REG)
    {
        // default register 1
        return od->reg1;
    }
    else if (od->type == EMPTY)
    {
        return 0;
    }
    else
    {
        // access memory: return the physical address
        uint64_t vaddr = 0;

        if (od->type == MEM_IMM)
        {
            vaddr = od->imm;
        }
        else if (od->type == MEM_REG1)
        {
            vaddr = *((uint64_t *)od->reg1);
        }
        else if (od->type == MEM_IMM_REG1)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg1));
        }
        else if (od->type == MEM_REG1_REG2)
        {
            vaddr = (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2));
        }
        else if (od->type == MEM_IMM_REG1_REG2)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2));
        }
        else if (od->type == MEM_REG2_SCAL)
        {
            vaddr = (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == MEM_IMM_REG2_SCAL)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == MEM_REG1_REG2_SCAL)
        {
            vaddr = (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == MEM_IMM_REG1_REG2_SCAL)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2)) * od->scal;
        }
        return vaddr;
    }

    // empty
    return 0;
}

static my_trie_node_t *register_trie = NULL;
static my_trie_node_t *operator_trie = NULL;

static uint64_t parse_register(const char *str)
{
    if (register_trie == NULL)
    {
        init_t register_init_elements[72] = {
            {"%rax", (uint64_t)&cpu_reg.rax},
            {"%rbx", (uint64_t)&cpu_reg.rbx},
            {"%rcx", (uint64_t)&cpu_reg.rcx},
            {"%rdx", (uint64_t)&cpu_reg.rdx},
            {"%rsi", (uint64_t)&cpu_reg.rsi},
            {"%rdi", (uint64_t)&cpu_reg.rdi},
            {"%rbp", (uint64_t)&cpu_reg.rbp},
            {"%rsp", (uint64_t)&cpu_reg.rsp},
            {"%r8", (uint64_t)&cpu_reg.r8},
            {"%r9", (uint64_t)&cpu_reg.r9},
            {"%r10", (uint64_t)&cpu_reg.r10},
            {"%r11", (uint64_t)&cpu_reg.r11},
            {"%r12", (uint64_t)&cpu_reg.r12},
            {"%r13", (uint64_t)&cpu_reg.r13},
            {"%r14", (uint64_t)&cpu_reg.r14},
            {"%r15", (uint64_t)&cpu_reg.r15},
            {"%eax", (uint64_t) & (cpu_reg.eax)},
            {"%ebx", (uint64_t) & (cpu_reg.ebx)},
            {"%ecx", (uint64_t) & (cpu_reg.ecx)},
            {"%edx", (uint64_t) & (cpu_reg.edx)},
            {"%esi", (uint64_t) & (cpu_reg.esi)},
            {"%edi", (uint64_t) & (cpu_reg.edi)},
            {"%ebp", (uint64_t) & (cpu_reg.ebp)},
            {"%esp", (uint64_t) & (cpu_reg.esp)},
            {"%r8d", (uint64_t) & (cpu_reg.r8d)},
            {"%r9d", (uint64_t) & (cpu_reg.r9d)},
            {"%r10d", (uint64_t) & (cpu_reg.r10d)},
            {"%r11d", (uint64_t) & (cpu_reg.r11d)},
            {"%r12d", (uint64_t) & (cpu_reg.r12d)},
            {"%r13d", (uint64_t) & (cpu_reg.r13d)},
            {"%r14d", (uint64_t) & (cpu_reg.r14d)},
            {"%r15d", (uint64_t) & (cpu_reg.r15d)},
            {"%ax", (uint64_t) & (cpu_reg.ax)},
            {"%bx", (uint64_t) & (cpu_reg.bx)},
            {"%cx", (uint64_t) & (cpu_reg.cx)},
            {"%dx", (uint64_t) & (cpu_reg.dx)},
            {"%si", (uint64_t) & (cpu_reg.si)},
            {"%di", (uint64_t) & (cpu_reg.di)},
            {"%bp", (uint64_t) & (cpu_reg.bp)},
            {"%sp", (uint64_t) & (cpu_reg.sp)},
            {"%r8w", (uint64_t) & (cpu_reg.r8w)},
            {"%r9w", (uint64_t) & (cpu_reg.r9w)},
            {"%r10w", (uint64_t) & (cpu_reg.r10w)},
            {"%r11w", (uint64_t) & (cpu_reg.r11w)},
            {"%r12w", (uint64_t) & (cpu_reg.r12w)},
            {"%r13w", (uint64_t) & (cpu_reg.r13w)},
            {"%r14w", (uint64_t) & (cpu_reg.r14w)},
            {"%r15w", (uint64_t) & (cpu_reg.r15w)},
            {"%ah", (uint64_t) & (cpu_reg.ah)},
            {"%bh", (uint64_t) & (cpu_reg.bh)},
            {"%ch", (uint64_t) & (cpu_reg.ch)},
            {"%dh", (uint64_t) & (cpu_reg.dh)},
            {"%sih", (uint64_t) & (cpu_reg.sih)},
            {"%dih", (uint64_t) & (cpu_reg.dih)},
            {"%bph", (uint64_t) & (cpu_reg.bph)},
            {"%sph", (uint64_t) & (cpu_reg.sph)},
            {"%r8w", (uint64_t) & (cpu_reg.r8b)},
            {"%r9w", (uint64_t) & (cpu_reg.r9b)},
            {"%r10w", (uint64_t) & (cpu_reg.r10b)},
            {"%r11w", (uint64_t) & (cpu_reg.r11b)},
            {"%r12w", (uint64_t) & (cpu_reg.r12b)},
            {"%r13w", (uint64_t) & (cpu_reg.r13b)},
            {"%r14w", (uint64_t) & (cpu_reg.r14b)},
            {"%r15w", (uint64_t) & (cpu_reg.r15b)},
            {"%al", (uint64_t) & (cpu_reg.al)},
            {"%bl", (uint64_t) & (cpu_reg.bl)},
            {"%cl", (uint64_t) & (cpu_reg.cl)},
            {"%dl", (uint64_t) & (cpu_reg.dl)},
            {"%sil", (uint64_t) & (cpu_reg.sil)},
            {"%dil", (uint64_t) & (cpu_reg.dil)},
            {"%bpl", (uint64_t) & (cpu_reg.bpl)},
            {"%spl", (uint64_t) & (cpu_reg.spl)},
        };
        init_register_tree(&register_trie, register_init_elements, sizeof(register_init_elements) / sizeof(init_t));
    }

    return register_address(register_trie, str);
}

static int parse_operator(const char *str)
{
    if (operator_trie == NULL)
    {
        init_t operator_init_elements[13] = {
            {"mov", 0},
            {"push", 1},
            {"pop", 2},
            {"leaveq", 3},
            {"callq", 4},
            {"retq", 5},
            {"add", 6},
            {"sub", 7},
            {"cmpq", 8},
            {"jne", 9},
            {"jmp", 10},
            {"int", 11}};

        init_operator_tree(&operator_trie, operator_init_elements, sizeof(operator_init_elements) / sizeof(init_t));
    }

    return operator_type(operator_trie, str);
}

void TestParseOperand()
{
    const char *strs[11] = {
        "$0x1234",
        "%rax",
        "0xabcd",
        "(%rsp)",
        "0xabcd(%rsp)",
        "(%rsp,%rbx)",
        "0xabcd(%rsp,%rbx)",
        "(,%rbx,8)",
        "0xabcd(,%rbx,8)",
        "(%rsp,%rbx,8)",
        "0xabcd(%rsp,%rbx,8)",
    };

    printf("rax %p\n", &(cpu_reg.rax));
    printf("rsp %p\n", &(cpu_reg.rsp));
    printf("rbx %p\n", &(cpu_reg.rbx));

    for (int i = 0; i < 11; ++i)
    {
        od_t od;
        parse_operand(strs[i], &od);

        // printf("\n%s\n", strs[i]);
        // printf("od enum type: %d\n", od.type);
        // printf("od imm: %lx\n", od.imm);
        // printf("od reg1: %lx\n", od.reg1);
        // printf("od reg2: %lx\n", od.reg2);
        // printf("od scal: %lx\n", od.scal);
    }
}

void TestParseInstruction()
{
    char assembly[15][MAX_INSTRUCTION_CHAR] = {
        "push   %rbp",                    // 0
        "mov    %rsp,%rbp",               // 1
        "mov    %rdi,-0x18(%rbp)",        // 2
        "mov    %rsi,-0x20(%rbp)",        // 3
        "mov    -0x18(%rbp,%rax,8),%rdx", // 4
        "mov    -0x20(%rbp),%rax",        // 5
        "add    %rdx,%rax",               // 6
        "mov    %rax,-0x8(%rbp)",         // 7
        "mov    -0x8(%rbp),%rax",         // 8
        "pop    %rbp",                    // 9
        "retq",                           // 10
        "mov    %rdx,%rsi",               // 11
        "mov    %rax,%rdi",               // 12
        "callq  0",                       // 13
        "mov    %rax,-0x8(%rbp)",         // 14
    };

    inst_t inst;
    for (int i = 0; i < 15; ++i)
    {
        parse_instruction(assembly[i], &inst);
    }
}

/*======================================*/
/*      instruction handlers            */
/*======================================*/

// insturction (sub)set
// In this simulator, the instructions have been decoded and fetched
// so there will be no page fault during fetching
// otherwise the instructions must handle the page fault (swap in from disk) first
// and then re-fetch the instruction and do decoding
// and finally re-run the instruction

static void mov_handler(od_t *src_od, od_t *dst_od);
static void push_handler(od_t *src_od, od_t *dst_od);
static void pop_handler(od_t *src_od, od_t *dst_od);
static void leave_handler(od_t *src_od, od_t *dst_od);
static void call_handler(od_t *src_od, od_t *dst_od);
static void ret_handler(od_t *src_od, od_t *dst_od);
static void add_handler(od_t *src_od, od_t *dst_od);
static void sub_handler(od_t *src_od, od_t *dst_od);
static void cmp_handler(od_t *src_od, od_t *dst_od);
static void jne_handler(od_t *src_od, od_t *dst_od);
static void jmp_handler(od_t *src_od, od_t *dst_od);
static void int_handler(od_t *src_od, od_t *dst_od);

// handler table storing the handlers to different instruction types
typedef void (*handler_t)(od_t *, od_t *);
// look-up table of pointers to function
static handler_t handler_table[NUM_INSTRTYPE] = {
    &mov_handler,   // 0
    &push_handler,  // 1
    &pop_handler,   // 2
    &leave_handler, // 3
    &call_handler,  // 4
    &ret_handler,   // 5
    &add_handler,   // 6
    &sub_handler,   // 7
    &cmp_handler,   // 8
    &jne_handler,   // 9
    &jmp_handler,   // 10
    &int_handler,
};

// update the rip pointer to the next instruction sequentially
static inline void increase_pc()
{
    // we are handling the fixed-length of assembly string here
    // but their size can be variable as true X86 instructions
    // that's because the operands' sizes follow the specific encoding rule
    // the risc-v is a fixed length ISA
    cpu_pc.rip = cpu_pc.rip + sizeof(char) * MAX_INSTRUCTION_CHAR;
}

static inline void reset_condition_flags()
{
    cpu_flags.CF = 0;
    cpu_flags.OF = 0;
    cpu_flags.SF = 0;
    cpu_flags.ZF = 0;
}

// instruction handlers
static void mov_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    uint64_t dst = compute_operand(dst_od);

    // mov %rax, %rbx
    if (src_od->type == REG && dst_od->type == REG)
    {
        *(uint64_t *)dst = *(uint64_t *)src;

        reset_condition_flags();
        increase_pc();
    }
    // mov %rax, 0x8(%rbx)
    else if ((src_od->type == REG && dst_od->type == MEM_IMM_REG1) || (src_od->type == REG && dst_od->type == MEM_IMM))
    {
        uint64_t value = *(uint64_t *)src;
        cpu_write64bits_dram(va2pa(dst, 1), value);

        reset_condition_flags();
        increase_pc();
    }
    // mov 0x8(%rax), %rbx
    else if (src_od->type == MEM_IMM_REG1 && dst_od->type == REG)
    {
        uint64_t value = cpu_read64bits_dram(va2pa(src, 0));
        *(uint64_t *)dst = value;

        reset_condition_flags();
        increase_pc();
    }
    else if (src_od->type == IMM && dst_od->type == REG)
    {
        *(uint64_t *)dst = src;
        reset_condition_flags();
        increase_pc();
    }
    else
    {
        printf("unsupport operand type, src = %d, dst = %d\n", src_od->type, dst_od->type);
    }
}

static void push_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    // uint64_t dst = compute_operand(dst_od);

    // push   %rbp
    if (src_od->type == REG)
    {
        // src: register
        // dst: empty
        uint64_t tmp_rsp = cpu_reg.rsp - 8;
        uint64_t paddr = va2pa(tmp_rsp, 1);

        // printf("after rsp = %lx\n", cpu_reg.rsp);
        uint64_t value = *(uint64_t *)src;
        cpu_write64bits_dram(paddr, value);

        // printf("paddr = %lx\n", paddr);
        // for (int i = 0; i < 7; ++i)
        // {
        //     // print as yellow
        //     printf(YELLOWSTR("%c"), pm[va2pa(tmp_rsp, 0) + i]);
        // }

        cpu_reg.rsp = tmp_rsp;
        reset_condition_flags();
        increase_pc();
    }
    else
    {
        printf("unsupport operand type, src = %d, dst = %d\n", src_od->type, dst_od->type);
    }
}

static void pop_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    // uint64_t dst = compute_operand(dst_od);

    if (src_od->type == REG)
    {
        uint64_t value = cpu_read64bits_dram(va2pa(cpu_reg.rsp, 0));
        *(uint64_t *)src = value;
        cpu_reg.rsp = cpu_reg.rsp + 0x8;

        reset_condition_flags();
        increase_pc();
    }
    else
    {
        printf("unsupport operand type, src = %d, dst = %d\n", src_od->type, dst_od->type);
    }
}

static void ret_handler(od_t *src_od, od_t *dst_od)
{
    // src: empty
    // dst: empty
    uint64_t value = cpu_read64bits_dram(va2pa(cpu_reg.rsp, 0));
    cpu_reg.rsp = cpu_reg.rsp + 0x8;
    cpu_pc.rip = value;

    reset_condition_flags();
}

static void add_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    uint64_t dst = compute_operand(dst_od);

    // add    %rdx,%rax
    if (src_od->type == REG && dst_od->type == REG)
    {
        *(uint64_t *)dst = *(uint64_t *)src + *(uint64_t *)dst;

        uint64_t val = *(uint64_t *)dst + *(uint64_t *)src;

        int val_sign = ((val >> 63) & 0x1);
        int src_sign = ((*(uint64_t *)src >> 63) & 0x1);
        int dst_sign = ((*(uint64_t *)dst >> 63) & 0x1);

        // 无符号溢出
        cpu_flags.CF = val < *(uint64_t *)src;
        cpu_flags.ZF = val == 0;
        cpu_flags.SF = val_sign == 1;
        // 有符号溢出条件：src dst 符号一样，与 val 符号不一样
        cpu_flags.OF = (src_sign == 0 && dst_sign == 0 && val_sign == 1) || (src_sign == 1 && dst_sign == 1 && val_sign == 0);
        // cpu_flags.OF = !(dst_sign ^ src_sign) && (src_sign ^ val_sign);

        increase_pc();
    }
    else
    {
        printf("unsupport operand type, src = %d, dst = %d\n", src_od->type, dst_od->type);
    }
}

static void call_handler(od_t *src_od, od_t *dst_od)
{

    uint64_t src = compute_operand(src_od);
    // uint64_t dst = compute_operand(dst_od);

    // src: immediate number: virtual address of target function starting
    // dst: empty
    // push the return value
    cpu_reg.rsp = cpu_reg.rsp - 8;

    // next inst addr
    uint64_t value = cpu_pc.rip + sizeof(char) * MAX_INSTRUCTION_CHAR;
    cpu_write64bits_dram(
        va2pa(cpu_reg.rsp, 1),
        value);
    // jump to target function address
    cpu_pc.rip = src;

    reset_condition_flags();
}

static void leave_handler(od_t *src_od, od_t *dst_od)
{
    /*
    Leave等价于（还原栈帧）：
        movl %ebp %esp
        popl %ebp
    */

    cpu_reg.rsp = cpu_reg.rbp;
    uint64_t old_value = cpu_read64bits_dram(va2pa(cpu_reg.rsp, 0));
    cpu_reg.rsp += 8;
    cpu_reg.rbp = old_value;

    reset_condition_flags();
    increase_pc();
}

static void sub_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    uint64_t dst = compute_operand(dst_od);
    if (src_od->type == IMM && dst_od->type == REG)
    {
        // 这里不能直接用减法，如果 src 实际上是一个负数，那么就会有问题???
        *(uint64_t *)dst = *(uint64_t *)dst + (~src + 1);
        // *(uint64_t *)dst = *(uint64_t *)dst - src;

        uint64_t val = *(uint64_t *)dst + (~src + 1);
        int val_sign = ((val >> 63) & 0x1);
        int src_sign = ((src >> 63) & 0x1);
        int dst_sign = ((*(uint64_t *)dst >> 63) & 0x1);

        // set condition flags
        cpu_flags.CF = val > *(uint64_t *)dst;
        cpu_flags.ZF = val == 0;
        cpu_flags.SF = val_sign == 1;
        cpu_flags.OF = (src_sign == 1 && dst_sign == 0 && val_sign == 1) || (src_sign == 0 && dst_sign == 1 && val_sign == 0);

        increase_pc();
    }
    else
    {
        printf("unsupport operand type, src = %d, dst = %d\n", src_od->type, dst_od->type);
    }
}

static void cmp_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    uint64_t dst = compute_operand(dst_od);
    if (src_od->type == IMM && dst_od->type == MEM_IMM_REG1)
    {
        // cmpq   $0x1,-0x8(%rbp)
        // 实际上的比较顺序是 -0x8(%rbp) ：$0x1
        // cmp 指令根据两个操作数之差来设置条件码。除了只设置条件码而不更新目的寄存器，cmp 与 sub 行为一样
        uint64_t dst_val = cpu_read64bits_dram(va2pa(dst, 0));
        uint64_t val = dst_val + (~src + 1);

        int val_sign = ((val >> 63) & 0x1);
        int src_sign = ((src >> 63) & 0x1);
        int dst_sign = ((dst_val >> 63) & 0x1);

        // set condition flags
        cpu_flags.CF = (val > dst_val); // unsigned
        cpu_flags.ZF = (val == 0);
        cpu_flags.SF = val_sign;
        cpu_flags.OF = (src_sign == 1 && dst_sign == 0 && val_sign == 1) || (src_sign == 0 && dst_sign == 1 && val_sign == 0);

        increase_pc();
    }
    else if (src_od->type == IMM && dst_od->type == REG)
    {
        // cmpq   $0x1,-0x8(%rbp)
        // 实际上的比较顺序是 -0x8(%rbp) ：$0x1
        // cmp 指令根据两个操作数之差来设置条件码。除了只设置条件码而不更新目的寄存器，cmp 与 sub 行为一样
        uint64_t dst_val = *(uint64_t *)dst;
        uint64_t val = dst_val + (~src + 1);

        int val_sign = ((val >> 63) & 0x1);
        int src_sign = ((src >> 63) & 0x1);
        int dst_sign = ((dst_val >> 63) & 0x1);

        // set condition flags
        cpu_flags.CF = (val > dst_val); // unsigned
        cpu_flags.ZF = (val == 0);
        cpu_flags.SF = val_sign;
        cpu_flags.OF = (src_sign == 1 && dst_sign == 0 && val_sign == 1) || (src_sign == 0 && dst_sign == 1 && val_sign == 0);

        increase_pc();
    }
    else
    {
        printf("unsupport operand type, src = %d, dst = %d\n", src_od->type, dst_od->type);
    }
}

static void jne_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);

    if (cpu_flags.ZF == 0)
    {
        // jmp to src address
        cpu_pc.rip = src;
    }
    else
    {
        // fetch next inst
        increase_pc();
    }
    reset_condition_flags();
}

static void jmp_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    cpu_pc.rip = src;
    reset_condition_flags();
}

// time, the craft of god
static uint64_t global_time = 0;
static uint64_t timer_period = 50000;

// instruction cycle is implemented in CPU
// the only exposed interface outside CPU
void instruction_cycle()
{
    // this is the entry point of the re-execution of
    // interrupt return instruction.
    // When a new process is scheduled, the first instruction/
    // return instruction should start here, jumping out of the
    // call stack of old process.
    // This is especially useful for page fault handling.
    setjmp(USER_INSTRUCTION_ON_IRET);

    global_time += 1;

    // FETCH: get the instruction string by program counter
    char inst_str[MAX_INSTRUCTION_CHAR];
    cpu_readinst_dram(va2pa(cpu_pc.rip, 0), inst_str);

    my_log(DEBUG_INSTRUCTIONCYCLE, "%8lx    %s\n", cpu_pc.rip, inst_str);

    // DECODE: decode the run-time instruction operands
    inst_t inst;
    parse_instruction(inst_str, &inst);

    // EXECUTE: get the function pointer or handler by the operator
    handler_t handler = handler_table[inst.op];
    // update CPU and memory according the instruction
    printf("%s\n", inst_str);
    handler(&(inst.src), &(inst.dst));

    // check timer interrupt from APIC
    if ((global_time % timer_period) == 0)
    {
        interrupt_stack_switching(0x81);
    }
}

void print_register()
{
    // if ((DEBUG_VERBOSE_SET & DEBUG_REGISTERS) == 0x0)
    // {
    //     return;
    // }

    printf("rax = %16lx\trbx = %16lx\trcx = %16lx\trdx = %16lx\n",
           cpu_reg.rax, cpu_reg.rbx, cpu_reg.rcx, cpu_reg.rdx);
    printf("rsi = %16lx\trdi = %16lx\trbp = %16lx\trsp = %16lx\n",
           cpu_reg.rsi, cpu_reg.rdi, cpu_reg.rbp, cpu_reg.rsp);
    printf("rip = %16lx\n", cpu_pc.rip);
    printf("CF = %u\tZF = %u\tSF = %u\tOF = %u\n",
           cpu_flags.CF, cpu_flags.ZF, cpu_flags.SF, cpu_flags.OF);
}

void print_stack()
{
    if ((DEBUG_VERBOSE_SET & DEBUG_PRINTSTACK) == 0x0)
    {
        return;
    }

    int n = 10;
    uint64_t *high = (uint64_t *)&pm[va2pa(cpu_reg.rsp, 0)];
    high = &high[n];
    uint64_t va = cpu_reg.rsp + n * 8;

    for (int i = 0; i < 2 * n; ++i)
    {
        uint64_t *ptr = (uint64_t *)(high - i);
        printf("0x%16lx : %16lx", va, (uint64_t)*ptr);

        if (i == n)
        {
            printf(" <== rsp");
        }
        printf("\n");
        va -= 8;
    }
}

void int_handler(od_t *src_od, od_t *dst_od)
{
    if (src_od->type == IMM)
    {
        // src: interrupt vector

        // Be careful here. Think why we need to increase RIP before interrupt?
        // This `int` instruction is executed by process 1,
        // but interrupt will cause OS's scheduling to process 2.
        // So this `int_handler` will not return.
        // When the execution of process 1 resumed, the system call is finished.
        // We want to execute the next instruction, so RIP pushed to trap frame
        // must be the next instruction.
        increase_pc();
        cpu_flags.__flags_value = 0;

        // This function will not return.
        interrupt_stack_switching(src_od->imm);
    }
}