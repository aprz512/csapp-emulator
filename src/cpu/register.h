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
} reg_t;

// fix error  register_t already used
// /Library/Developer/CommandLineTools/SDKs/MacOSX12.3.sdk/usr/include/i386/types.h:90:33: note: previous definition is here
// typedef int64_t                 register_t;
// 全局只有一个 register
reg_t reg;

#endif