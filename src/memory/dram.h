#ifndef dram_header
#define dram_header

#include <stdint.h>

#define MEMOERY_LEN 1000

uint8_t physical_memory[MEMOERY_LEN]; // physical memory

uint64_t read64bits_dram(uint64_t paddr);
void write64bits_dram(uint64_t paddr, uint64_t data);

#endif