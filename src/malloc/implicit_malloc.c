
#include "headers/allocator.h"
#include <stdio.h>

extern uint8_t heap[MAX_HEAP_SIZE];
extern uint64_t prologue_payload;
extern uint64_t epilogue_payload; // epilogue 没有 payload，让它指向 header 的最后
extern uint64_t mem_brk;

uint64_t merge_free(uint64_t payload_addr)
{
    uint32_t size = get_block_size(payload_addr);

    uint64_t next_block_payload_addr = next_block_payload(payload_addr);
    uint32_t next_block_size = get_block_size(next_block_payload_addr);
    uint32_t next_alloc = get_block_alloc(next_block_payload_addr);

    uint64_t pre_block_payload_addr = pre_block_payload(payload_addr);
    uint32_t pre_block_size = get_block_size(pre_block_payload_addr);
    uint32_t pre_alloc = get_block_alloc(pre_block_payload_addr);

    if (next_alloc == 1 && pre_alloc == 1)
    {
        // no merge
        return payload_addr;
    }
    else if (next_alloc == 1 && pre_alloc == 0)
    {
        // pre merge
        set_block_header(pre_block_payload_addr, pack(size + pre_block_size, 0));
        set_block_footer(payload_addr, pack(size + pre_block_size, 0));
        return pre_block_payload_addr;
    }
    else if (next_alloc == 0 && pre_alloc == 1)
    {
        // next merge
        set_block_header(payload_addr, pack(size + next_block_size, 0));
        set_block_header(next_block_payload_addr, pack(size + next_block_size, 0));
        return payload_addr;
    }
    else
    {
        // merge pre and next
        set_block_header(pre_block_payload_addr, pack(size + pre_block_size + next_block_size, 0));
        set_block_header(next_block_payload_addr, pack(size + pre_block_size + next_block_size, 0));
        return pre_block_payload_addr;
    }
}

uint64_t extend_heap(uint32_t words)
{
    // 做一个对齐操作，如是 words 是奇数，就加一个 WSIZE 的空间
    uint32_t size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    uint32_t extend_block_payload = mm_brk(size);
    set_block_header(extend_block_payload, pack(size, 0));
    set_block_footer(extend_block_payload, pack(size, 0));

    epilogue_payload = next_block_payload(extend_block_payload);
    set_block_header(epilogue_payload, pack(0, 1));

    return merge_free(extend_block_payload);
}

uint64_t place(uint64_t payload_addr, uint32_t align_size)
{
    uint32_t block_size = get_block_size(payload_addr);

    if (block_size > align_size)
    {
        set_block_header(payload_addr, pack(align_size, 1));
        set_block_footer(payload_addr, pack(align_size, 1));

        // 可能剩余的只有一个2个WSIZE了，那么就是内部碎片
        uint32_t split_block = next_block_payload(payload_addr);
        set_block_header(split_block, pack(block_size - align_size, 0));
        set_block_footer(split_block, pack(block_size - align_size, 0));

        return payload_addr;
    }
    else if (block_size == align_size)
    {
        set_block_header(payload_addr, pack(align_size, 1));
        set_block_footer(payload_addr, pack(align_size, 1));
        return payload_addr;
    }

    return 0;
}

int mm_init(void)
{

    // reset all to 0
    for (int i = 0; i < MAX_HEAP_SIZE / 8; i += 8)
    {
        *(uint64_t *)&heap[i] = 0;
    }

    // 至少可以分配4个字，給 prologue 与 epilogye
    // 一个字用来对齐
    // prologue 占据两个字
    // epilogye 占据一个字
    uint32_t heap_listp = mm_brk(4 * WSIZE);

    // 设置 prologue
    set_block_header(heap_listp + 2 * WSIZE, pack(DSIZE, 1));
    set_block_footer(heap_listp + WSIZE * 2, pack(DSIZE, 1));

    // 设置 epilogye，结尾块不需要 footer，
    // 而且我们也将 block 的 size 设置为 0，虽然它实际上占了一个字
    set_block_header(heap_listp + WSIZE * 3, pack(0, 1));

    // 让堆指针指向 prologue block 的中间
    prologue_payload = 2 * WSIZE;

    // 初始，分配一个大的 free block，为 1<<12 个字节
    extend_heap(CHUNKSIZE / WSIZE);
}

uint64_t mm_malloc(uint32_t size)
{

    uint32_t align_size = 0;
    // 至少要16个字节，而且是 8 的倍数
    if (size < DSIZE)
    {
        align_size = 2 * DSIZE;
    }
    else
    {
        align_size = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
    }

    uint32_t next_block = get_prologue_payload();
    uint32_t next_block_size = get_block_size(next_block);

    while (next_block != get_epilogue_payload())
    {
        next_block = next_block_payload(next_block);
        next_block_size = get_block_size(next_block);

        // 可以分配这个空闲块
        if (get_block_alloc(next_block) == 0 && next_block_size >= align_size)
        {
            printf("try malloc heap!!!\n");
            return place(next_block, align_size);
        }
    }

    printf("need extend heap!!!\n");

    // 没找到空闲且大于需求 size 的 block
    uint32_t extend_size = align_size > CHUNKSIZE ? CHUNKSIZE : align_size;

    // 新分配的，肯定可以放的下
    uint64_t block_payload = extend_heap(extend_size / WSIZE);

    return place(block_payload, extend_size);
}

void mm_free(uint64_t payload_addr)
{
    uint32_t size = get_block_size(payload_addr);
    set_block_header(payload_addr, pack(size, 0));
    set_block_footer(payload_addr, pack(size, 0));

    merge_free(payload_addr);
}