#include "headers/mmu.h"
#include "headers/memory.h"
#include "headers/address.h"
#include "headers/cpu.h"
#include <stdlib.h>
#include <assert.h>

static uint64_t page_walk(uint64_t vaddr);
static void page_fault_handler(pte4_t *pte4, address_t vaddr);

/**
 * @brief 因为 vaddr 时连续的，所以使用这种方式计算出来的 vaddr 也是连续的，注意不要使用太多地址，导致覆盖就行
 *
 * @param vaddr
 * @return uint64_t
 */
uint64_t va2pa(uint64_t vaddr)
{
    return vaddr % PHYSICAL_MEMORY_SPACE;
}

static uint64_t page_walk(uint64_t vaddr_value)
{
    address_t vaddr = {
        .address_value = vaddr_value
    };

    int page_table_size = PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t);  // should be 4KB

    // CR3 register's value is malloced on the heap of the simulator
    pte123_t *pgd = (pte123_t *)cpu_controls.cr3;
    assert(pgd != NULL);
    
    if (pgd[vaddr.VPN1].present == 1)
    {
        // PHYSICAL PAGE NUMBER of the next level page table
        // aka. high bits starting address of the page table
        pte123_t *pud = pgd[vaddr.VPN1].paddr;

        if (pud[vaddr.VPN2].present == 1)
        {
            // find pmd ppn

            pte123_t *pmd = (pte123_t *)(pud[vaddr.VPN2].paddr);

            if (pmd[vaddr.VPN3].present == 1)
            {
                // find pt ppn
                
                pte4_t *pt = (pte4_t *)(pmd[vaddr.VPN3].paddr);

                if (pt[vaddr.VPN4].present == 1)
                {
                    // find page table entry
                    address_t paddr = {
                        .PPN= pt[vaddr.VPN4].ppn,
                        .PPO = vaddr.VPO    // page offset inside the 4KB page
                    };

                    return paddr.paddr_value;
                }
                else
                {
                    // page table entry not exists
                    printf("page walk level 4: pt[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
                    // search paddr from main memory and disk
                    // TODO: raise exception 14 (page fault) here
                    // switch privilege from user mode (ring 3) to kernel mode (ring 0)
                    page_fault_handler(&pt[vaddr.VPN4], vaddr);

                    /*
                    pte4_t *pt = malloc(page_table_size);
                    memset(pt, 0, page_table_size);

                    // set page table entry
                    pmd[vaddr.vpn3].present = 1;
                    pud[vaddr.vpn3].paddr   = (uint64_t)pt;

                    // TODO: page fault here
                    // map the physical page and the virtual page
                    exit(0);
                    */
                }
            }
            else
            {
                // pt - level 4 not exists
                printf("page walk level 3: pmd[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
                pte4_t *pt = malloc(page_table_size);
                memset(pt, 0, page_table_size);

                // set page table entry
                pmd[vaddr.VPN3].present = 1;
                pud[vaddr.VPN3].paddr   = (uint64_t)pt;

                // TODO: page fault here
                // map the physical page and the virtual page
                exit(0);
            }
        }
        else
        {
            // pmd - level 3 not exists
            printf("page walk level 2: pud[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
            pte123_t *pmd = malloc(page_table_size);
            memset(pmd, 0, page_table_size);

            // set page table entry
            pud[vaddr.VPN2].present = 1;
            pud[vaddr.VPN2].paddr   = (uint64_t)pmd;

            // TODO: page fault here
            // map the physical page and the virtual page
            exit(0);
        }
    }
    else
    {
        // pud - level 2 not exists
        printf("page walk level 1: pgd[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
        pte123_t *pud = malloc(page_table_size);
        memset(pud, 0, page_table_size);

        // set page table entry
        pgd[vaddr.VPN1].present = 1;
        pgd[vaddr.VPN1].paddr   = (uint64_t)pud;

        // TODO: page fault here
        // map the physical page and the virtual page
        exit(0);
    }
}

static void page_fault_handler(pte4_t *pte, address_t vaddr)
{
    // select one victim physical page to swap to disk
    assert(pte->present == 0);

    // this is the selected ppn for vaddr
    int ppn = -1;
    pte4_t *victim = NULL;
    uint64_t daddr = 0xffffffffffffffff;

    // 1. try to request one free physical page from DRAM
    // kernel's responsibility
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
    {
        if (page_map[i].pte4->present == 0)
        {
            printf("PageFault: use free ppn %d\n", i);

            // found i as free ppn
            ppn = i;

            page_map[ppn].allocated = 1;    // allocate for vaddr
            page_map[ppn].dirty = 0;    // allocated as clean
            page_map[ppn].time = 0;    // most recently used physical page
            page_map[ppn].pte4 = pte;

            pte->present = 1;
            pte->ppn = ppn;
            pte->dirty = 0;

            return;
        }
    }

    // 2. no free physical page: select one clean page (LRU) and overwrite
    // in this case, there is no DRAM - DISK transaction
    int lru_ppn = -1;
    int lru_time = -1;
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
    {
        if (page_map[i].dirty == 0 && 
            lru_time < page_map[i].time)
        {
            lru_time = page_map[i].time;
            lru_ppn = i;
        }
    }
    
    if (-1 != lru_ppn && lru_ppn < MAX_NUM_PHYSICAL_PAGE)
    {
        ppn = lru_ppn;

        // reversed mapping
        victim = page_map[ppn].pte4;

        victim->pte_value = 0;
        victim->present = 0;
        victim->swap_id = page_map[ppn].swap_id;

        // load page from disk to physical memory first
        daddr = pte->swap_id;
        swap_in(pte->swap_id, ppn);

        pte->pte_value = 0;
        pte->present = 1;
        pte->ppn = ppn;
        pte->dirty = 0;

        page_map[ppn].allocated = 1;
        page_map[ppn].time = 0;
        page_map[ppn].dirty = 0;
        page_map[ppn].pte4 = pte;
        page_map[ppn].swap_id = daddr;

        return;
    }

    // 3. no free nor clean physical page: select one LRU victim
    // write back (swap out) the DIRTY victim to disk
    lru_ppn = -1;
    lru_time = -1;
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
    {
        if (lru_time < page_map[i].time)
        {
            lru_time = page_map[i].time;
            lru_ppn = i;
        }
    }

    assert(0 <= lru_ppn && lru_ppn < MAX_NUM_PHYSICAL_PAGE);

    ppn = lru_ppn;

    // reversed mapping
    victim = page_map[ppn].pte4;

    // write back
    swap_out(page_map[ppn].swap_id, ppn);

    victim->pte_value = 0;
    victim->present = 0;
    victim->swap_id = page_map[ppn].swap_id;

    // load page from disk to physical memory first
    daddr = pte->swap_id;
    swap_in(daddr, ppn);

    pte->pte_value = 0;
    pte->present = 1;
    pte->ppn = ppn;
    pte->dirty = 0;

    page_map[ppn].allocated = 1;
    page_map[ppn].time = 0;
    page_map[ppn].dirty = 0;
    page_map[ppn].pte4 = pte;
    page_map[ppn].swap_id = daddr;
}