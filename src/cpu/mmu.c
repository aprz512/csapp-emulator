#include "headers/mmu.h"
#include "headers/memory.h"
#include "headers/address.h"
#include "headers/cpu.h"
#include "headers/color.h"
#include "headers/interrupt.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static uint64_t page_walk(uint64_t vaddr_value, int write_request);
// static void page_fault_handler(pte4_t *pte4, address_t vaddr);

/**
 * @brief 因为 vaddr 时连续的，所以使用这种方式计算出来的 vaddr 也是连续的，注意不要使用太多地址，导致覆盖就行
 *
 * @param vaddr
 * @return uint64_t
 */
uint64_t va2pa(uint64_t vaddr, int write_request)
{
    // return vaddr % PHYSICAL_MEMORY_SPACE;
    return page_walk(vaddr, write_request);
}

static uint64_t page_walk(uint64_t vaddr_value, int write_request)
{
    // parse address
    address_t vaddr = {
        .vaddr_value = vaddr_value
    };
    int vpns[4] = {
        vaddr.vpn1,
        vaddr.vpn2,
        vaddr.vpn3,
        vaddr.vpn4,
    };
    int vpo = vaddr.vpo;

    int page_table_size = PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t);

    // CR3 register's value is malloced on the heap of the simulator
    pte123_t *pgd = (pte123_t *)cpu_controls.cr3;
    assert(pgd != NULL);
    assert(sizeof(pte123_t) == sizeof(pte4_t));
    assert(page_table_size == (1 << 12));

    int level = 0;
    pte123_t *tab = pgd;
    while (level < 3)
    {
        int vpn = vpns[level];
        if (tab[vpn].present != 1)
        {
            // page fault
            printf(REDSTR("MMU (%lx): level %d page fault: [%x].present == 0\n"), vaddr_value, level + 1, vpn);
            goto RAISE_PAGE_FAULT;
        }

        // move to next level
        tab = (pte123_t *)((uint64_t)tab[vpn].paddr);
        level += 1;
    }

    pte4_t *pte = &((pte4_t *)tab)[vaddr.vpn4];
    if (pte->present == 1)
    {
        // find page table entry
        address_t paddr = {
            .ppn = pte->ppn,
            .ppo = vpo    // page offset inside the 4KB page
        };

        if (pte->readonly == 1 && write_request)
        {
            // actually protection fault
            printf(REDSTR("\tProtection Fault\n"));
            goto RAISE_PAGE_FAULT;
        }

        return paddr.paddr_value;
    }
    else
    {
        printf(REDSTR("MMU (%lx): level 4 page fault: [%x].present == 0\n"), vaddr_value, vaddr.vpn4);
    }

RAISE_PAGE_FAULT:
    mmu_vaddr_pagefault = vaddr.address_value;
    // This interrupt will not return
    interrupt_stack_switching(0x0e);
    return 0;
}

// static void page_fault_handler(pte4_t *pte, address_t vaddr)
// {
//     // select one victim physical page to swap to disk
//     assert(pte->present == 0);

//     // this is the selected ppn for vaddr
//     int ppn = -1;
//     pte4_t *victim = NULL;
//     uint64_t daddr = 0xffffffffffffffff;

//     // 1. try to request one free physical page from DRAM
//     // kernel's responsibility
//     for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
//     {
//         if (page_map[i].pte4->present == 0)
//         {
//             printf("PageFault: use free ppn %d\n", i);

//             // found i as free ppn
//             ppn = i;

//             page_map[ppn].allocated = 1;    // allocate for vaddr
//             page_map[ppn].dirty = 0;    // allocated as clean
//             page_map[ppn].time = 0;    // most recently used physical page
//             page_map[ppn].pte4 = pte;

//             pte->present = 1;
//             pte->ppn = ppn;
//             pte->dirty = 0;

//             return;
//         }
//     }

//     // 2. no free physical page: select one clean page (LRU) and overwrite
//     // in this case, there is no DRAM - DISK transaction
//     int lru_ppn = -1;
//     int lru_time = -1;
//     for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
//     {
//         if (page_map[i].dirty == 0 && 
//             lru_time < page_map[i].time)
//         {
//             lru_time = page_map[i].time;
//             lru_ppn = i;
//         }
//     }
    
//     if (-1 != lru_ppn && lru_ppn < MAX_NUM_PHYSICAL_PAGE)
//     {
//         ppn = lru_ppn;

//         // reversed mapping
//         victim = page_map[ppn].pte4;

//         victim->pte_value = 0;
//         victim->present = 0;
//         victim->saddr = page_map[ppn].saddr;

//         // load page from disk to physical memory first
//         daddr = pte->saddr;
//         swap_in(pte->saddr, ppn);

//         pte->pte_value = 0;
//         pte->present = 1;
//         pte->ppn = ppn;
//         pte->dirty = 0;

//         page_map[ppn].allocated = 1;
//         page_map[ppn].time = 0;
//         page_map[ppn].dirty = 0;
//         page_map[ppn].pte4 = pte;
//         page_map[ppn].saddr = daddr;

//         return;
//     }

//     // 3. no free nor clean physical page: select one LRU victim
//     // write back (swap out) the DIRTY victim to disk
//     lru_ppn = -1;
//     lru_time = -1;
//     for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
//     {
//         if (lru_time < page_map[i].time)
//         {
//             lru_time = page_map[i].time;
//             lru_ppn = i;
//         }
//     }

//     assert(0 <= lru_ppn && lru_ppn < MAX_NUM_PHYSICAL_PAGE);

//     ppn = lru_ppn;

//     // reversed mapping
//     victim = page_map[ppn].pte4;

//     // write back
//     swap_out(page_map[ppn].saddr, ppn);

//     victim->pte_value = 0;
//     victim->present = 0;
//     victim->saddr = page_map[ppn].saddr;

//     // load page from disk to physical memory first
//     daddr = pte->saddr;
//     swap_in(daddr, ppn);

//     pte->pte_value = 0;
//     pte->present = 1;
//     pte->ppn = ppn;
//     pte->dirty = 0;

//     page_map[ppn].allocated = 1;
//     page_map[ppn].time = 0;
//     page_map[ppn].dirty = 0;
//     page_map[ppn].pte4 = pte;
//     page_map[ppn].saddr = daddr;
// }