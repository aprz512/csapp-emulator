#include "headers/allocator.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t heap[MAX_HEAP_SIZE];
extern uint64_t prologue_payload;
extern uint64_t epilogue_payload; // epilogue 没有 payload，让它指向 header 的最后
extern uint64_t mem_brk;

uint32_t get_block_alloc(uint64_t payload_addr)
{
    uint32_t header_value = *(uint32_t *)&heap[payload_addr - WSIZE];
    return (header_value & (0x1));
}

uint32_t get_block_size(uint64_t payload_addr)
{
    uint32_t header_value = *(uint32_t *)&heap[payload_addr - WSIZE];
    uint32_t block_size = (header_value & (~0x7));

    return block_size;
}

uint32_t pack(uint32_t size, uint32_t alloc)
{
    return size | alloc;
}

void set_block_header(uint64_t payload_addr, uint32_t pack)
{
    *(uint32_t *)&heap[payload_addr - WSIZE] = pack;
}

void set_block_footer(uint64_t payload_addr, uint32_t pack)
{
    uint32_t size = get_block_size(payload_addr);
    *(uint32_t *)&heap[payload_addr - WSIZE + size - WSIZE] = pack;
}

uint64_t next_block_payload(uint64_t payload_addr)
{
    uint32_t size = get_block_size(payload_addr);
    return payload_addr + size;
}

uint64_t pre_block_payload(uint64_t payload_addr)
{
    return payload_addr - get_block_size(payload_addr - 2 * WSIZE);
}

uint64_t mm_brk(uint32_t size)
{
    uint32_t old = mem_brk;
    if (mem_brk + size > MAX_HEAP_SIZE)
    {
        printf("OS cannot allocate physical page for heap!\n");
        exit(0);
    }
    mem_brk += size;

    return old;
}

uint64_t get_first_block_payload()
{
    return next_block_payload(get_prologue_payload());
}

uint64_t get_prologue_payload()
{
    return prologue_payload;
}

uint64_t get_last_block_payload()
{
    return pre_block_payload(get_prologue_payload());
}

uint64_t get_epilogue_payload()
{
    return epilogue_payload;
}

uint64_t get_block_next_ptr(uint64_t payload_addr)
{
    return payload_addr + WSIZE;
}

uint64_t get_block_pre_ptr(uint64_t payload_addr)
{
    return payload_addr;
}

uint64_t set_block_next_ptr(uint64_t payload_addr, uint64_t next)
{
    *(uint32_t *)&heap[get_block_next_ptr(payload_addr)] = next & 0xFFFFFFFF;
}

uint64_t set_block_pre_ptr(uint64_t payload_addr, uint64_t pre)
{
    *(uint32_t *)&heap[get_block_pre_ptr(payload_addr)] = pre & 0xFFFFFFFF;
}
