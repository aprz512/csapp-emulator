#ifndef register_header
#define register_header

#include <stdint.h>

typedef struct REGISTER
{

    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;

    // 程序计数器
    uint64_t rip;
} register_t;

// 全局只有一个 register
register_t reg;

#endif