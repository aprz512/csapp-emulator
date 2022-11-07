#ifndef COMMON_HEADER
#define COMMON_HEADER

#include <stdint.h>

// convert string dec or hex to the integer bitmap
uint64_t string2uint(const char *src);
uint64_t string2uint_range(const char *src, int start, int end);

// commonly shared variables
#define MAX_INSTRUCTION_CHAR 64

// do page walk
#define DEBUG_ENABLE_PAGE_WALK      0

// use sram cache for memory access 
#define DEBUG_ENABLE_SRAM_CACHE     0


#endif