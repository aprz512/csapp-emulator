/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include "headers/algorithm.h"
#include "headers/address.h"
#include "headers/instruction.h"
#include "headers/interrupt.h"
#include "headers/process.h"
#include "headers/color.h"

void map_pte4(pte4_t *pte, uint64_t ppn);
void unmapall_pte4(uint64_t ppn);
void page_map_init();
void pagemap_dirty(uint64_t ppn);
void pagemap_update_time(uint64_t ppn);
void set_pagemap_swapaddr(uint64_t ppn, uint64_t swap_address);
uint64_t allocate_swappage(uint64_t ppn);
pte123_t *get_pagetableentry(pte123_t *pgd, address_t *vaddr, int level, int allocate);

static void TestFork_cow()
{
    printf("================\nTesting fork <Copy On Write> ...\n");

    cpu_reg.rsp = 0x7ffffffee0f0;
    cpu_pc.rip = 0x00400000;
    address_t code_addr = {.address_value = cpu_pc.rip};
    address_t stack_addr = {.address_value = cpu_reg.rsp};
    
    page_map_init();

    // pcb is needed to trigger page fault
    pcb_t p1;
    memset(&p1, 0, sizeof(pcb_t));
    p1.pid = 1;
    // the next switched process would still be p1
    p1.next = &p1;
    p1.prev = &p1;

    // prepare vm areas instead of page tables
    // The memory loading should build the page tables from the vm areas
    vm_area_t vmas [2] = {
        // vm area for .text section
        {
            .vma_start = 0x00400000,
            .vma_end = 0x00401000,
            .vma_mode.read = 1,
            .vma_mode.write = 0,
            .vma_mode.execute = 1,
            .vma_mode.private = 1,
            .filepath = "~/fork"
        },
        // vm area for stack
        {
            .vma_start = ((cpu_reg.rsp) >> 12) << 12,
            .vma_end = (((cpu_reg.rsp) >> 12) + 1) << 12,
            .vma_mode.read = 1,
            .vma_mode.write = 1,
            .vma_mode.execute = 0,
            .vma_mode.private = 1,
            .filepath = "[stack]"
        },
    };

    vma_add_area(&p1, &vmas[0]);
    vma_add_area(&p1, &vmas[1]);
    setup_pagetable_from_vma(&p1);

    // load code to frame 0
    char code[22][MAX_INSTRUCTION_CHAR] = {
        // set PID = 0;
        "mov    $0x0,%rbx",     // 0x00400000
        // fork
        "mov    $0x39,%rax",
        "int    $0x80",
        // check fork result to detect parent or child
        "mov    %rax,%rbx",
        "cmpq   $0x0,%rbx",
        // not returns 0, then parent process
        "jne    $0x00400380",
        // child LOOP: print child
        "mov   $0x0a646c696863,%rbx",   // 0x00400180
        "push  %rbx",
        "mov   $1,%rax",
        "mov   $1,%rdi",
        "mov   %rsp,%rsi",
        "mov   $6,%rdx",
        "int    $0x80",
        "jmp    $0x00400200",
        // LOOP: parent
        // parent LOOP: print parent
        "mov   $0x000a746e65726170,%rbx", // 0x00400380
        "push  %rbx",
        "mov   $1,%rax",
        "mov   $1,%rdi",
        "mov   %rsp,%rsi",
        "mov   $7,%rdx",
        "int    $0x80",
        "jmp    $0x00400400",
    };

    uint64_t code_ppn = (uint64_t)(((pte4_t *)get_pagetableentry(p1.mm.pgd, &code_addr, 4, 0))->ppn);
    memcpy(
        (char *)(&pm[code_ppn]),
        &code, sizeof(char) * 22 * MAX_INSTRUCTION_CHAR);

    // create kernel stacks for trap into kernel
    kstack_t *stack_buf = aligned_alloc(KERNEL_STACK_SIZE, KERNEL_STACK_SIZE);
    uint64_t p1_stack_bottom = (uint64_t)stack_buf;
    p1.kstack = stack_buf;
    p1.kstack->threadinfo.pcb = &p1;

    // run p1
    tr_global_tss.ESP0 = p1_stack_bottom + KERNEL_STACK_SIZE;

    cpu_controls.cr3 = p1.mm.pgd_paddr;
    idt_init();
    syscall_init();

    // this should trigger page fault
    for (int i = 0; i < 100; ++i)
    {
        instruction_cycle();
    }

    printf(GREENSTR("Pass\n"));
}

int main()
{
    TestFork_cow();
    return 0;
}