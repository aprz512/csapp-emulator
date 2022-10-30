#ifndef COMMON_HEADER
#define COMMON_HEADER

#include <stdint.h>

// convert string dec or hex to the integer bitmap
uint64_t string2uint(const char *src);
uint64_t string2uint_range(const char *src, int start, int end);

#endif