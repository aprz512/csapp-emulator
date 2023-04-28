#ifndef ALLOCATOR_HEADER
#define ALLOCATOR_HEADER

#include <stdint.h>

#define MAX_HEAP_SIZE (8 * 4 * 1024)
#define WSIZE (4)
#define DSIZE (8)
#define CHUNKSIZE (1 << 12)

uint32_t get_block_alloc(uint64_t payload_addr);
uint32_t get_block_size(uint64_t payload_addr);
uint32_t pack(uint32_t size, uint32_t alloc);
void set_block_header(uint64_t payload_addr, uint32_t pack);
void set_block_footer(uint64_t payload_addr, uint32_t pack);
uint64_t next_block_payload(uint64_t payload_addr);
uint64_t pre_block_payload(uint64_t payload_addr);
uint64_t get_first_block_payload();
uint64_t get_last_block_payload();
uint64_t get_prologue_payload();
uint64_t get_epilogue_payload();
uint64_t get_block_next_ptr(uint64_t payload_addr);
uint64_t get_block_pre_ptr(uint64_t payload_addr);
uint64_t set_block_next_ptr(uint64_t payload_addr, uint64_t next);
uint64_t set_block_pre_ptr(uint64_t payload_addr, uint64_t pre);


uint64_t mm_brk(uint32_t size);
uint64_t merge_free(uint64_t payload_addr);
uint64_t extend_heap(uint32_t words);
uint64_t place(uint64_t payload_addr, uint32_t align_size);

int mm_init(void);
uint64_t mm_malloc(uint32_t size);
void mm_free(uint64_t payload_addr);

#endif