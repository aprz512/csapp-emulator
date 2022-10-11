#ifndef mmu_header
#define mmu_header

#include <stdint.h>

/**
 * @brief convert virtual address to phsyical address
 * 
 * @return uint64_t 
 */
uint64_t va2pa(uint64_t);

#endif