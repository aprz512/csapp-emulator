#ifndef DEBUG_HEADER
#define DEBUG_HEADER

#include <stdint.h>

#define DEBUG_ALL                   -0x1
#define DEBUG_INSTRUCTIONCYCLE      0x1
#define DEBUG_REGISTERS             0x1 << 1
#define DEBUG_PRINTSTACK            0x1 << 2
#define DEBUG_PRINTCACHESET         0x1 << 3
#define DEBUG_CACHEDETAILS          0x1 << 4
#define DEBUG_MMU                   0x1 << 5
#define DEBUG_LINKER                0x1 << 6
#define DEBUG_LOADER                0x1 << 7
#define DEBUG_PARSEINST             0x1 << 8
#define DEBUG_TRIE                  0x1 << 9

#define DEBUG_VERBOSE_SET           DEBUG_INSTRUCTIONCYCLE | DEBUG_MMU

uint64_t my_log(uint64_t open_set, const char *format, ...);

#endif