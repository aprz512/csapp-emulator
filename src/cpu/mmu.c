#include "headers/mmu.h"
#include "memory/dram.h"

/**
 * @brief 因为 vaddr 时连续的，所以使用这种方式计算出来的 vaddr 也是连续的，注意不要使用太多地址，导致覆盖就行
 * 
 * @param vaddr 
 * @return uint64_t 
 */
uint64_t va2pa(uint64_t vaddr) {
    return vaddr % MEMOERY_LEN;
}