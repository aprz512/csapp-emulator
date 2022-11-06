#ifndef MMU_HEADER
#define MMU_HEADER

#include <stdint.h>

/*--------------------------------------*/
// mmu functions
/*--------------------------------------*/

// translate the virtual address to physical address in MMU
// each MMU is owned by each core
uint64_t va2pa(uint64_t vaddr);

#endif