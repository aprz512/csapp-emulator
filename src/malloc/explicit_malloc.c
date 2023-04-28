
#include "headers/allocator.h"
#include <stdlib.h>
#include <stdio.h>

extern uint8_t heap[MAX_HEAP_SIZE];
extern uint64_t prologue_payload;
extern uint64_t epilogue_payload; // epilogue 没有 payload，让它指向 header 的最后
extern uint64_t mem_brk;


static int free_list_counter = 0;
static uint64_t free_list_header_payload = 0;

static void delete_block_from_free_list(uint64_t block_payload_addr)
{
    if (free_list_counter <= 0)
    {
        // should not reach here
        printf("should not reach here, wtf!!!");
        exit(0);
        return;
    }
    else if (free_list_counter == 1)
    {
        free_list_counter = 0;
        free_list_header_payload = 0;
    }
    else
    {
        uint64_t pre_payload = heap[get_block_pre_ptr(block_payload_addr)];
        uint64_t next_payload = heap[get_block_next_ptr(block_payload_addr)];

        set_block_next_ptr(pre_payload, next_payload);
        set_block_pre_ptr(next_payload, pre_payload);
        if (block_payload_addr == free_list_header_payload)
        {
            free_list_header_payload = pre_payload;
        }
        free_list_counter--;
    }
}

static void insert_block_from_free_list(uint64_t block_payload_addr)
{
    if (free_list_counter == 0)
    {
        // 链表的第一个元素
        set_block_next_ptr(block_payload_addr, block_payload_addr);
        set_block_pre_ptr(block_payload_addr, block_payload_addr);
    }
    else
    {
        uint64_t header_pre = get_block_next_ptr(free_list_header_payload);
        uint64_t tail_block_payload = heap[header_pre];

        set_block_next_ptr(tail_block_payload, block_payload_addr);
        set_block_pre_ptr(free_list_header_payload, block_payload_addr);

        set_block_next_ptr(block_payload_addr, free_list_header_payload);
        set_block_pre_ptr(block_payload_addr, tail_block_payload);
    }

    free_list_header_payload = block_payload_addr;
    free_list_counter++;
}

uint64_t merge_free(uint64_t payload_addr)
{
    uint32_t size = get_block_size(payload_addr);

    uint64_t next_block_payload_addr = next_block_payload(payload_addr);
    uint32_t next_block_size = get_block_size(next_block_payload_addr);
    uint32_t next_alloc = get_block_alloc(next_block_payload_addr);

    uint64_t pre_block_payload_addr = pre_block_payload(payload_addr);
    uint32_t pre_block_size = get_block_size(pre_block_payload_addr);
    uint32_t pre_alloc = get_block_alloc(pre_block_payload_addr);

    printf("%d - %d \n", next_alloc, pre_alloc);

    if (next_alloc == 1 && pre_alloc == 1)
    {
        insert_block_from_free_list(payload_addr);
        printf("no merge\n");
        // no merge
        return payload_addr;
    }
    else if (next_alloc == 1 && pre_alloc == 0)
    {
        printf("pre merge\n");
        // pre merge
        set_block_header(pre_block_payload_addr, pack(size + pre_block_size, 0));
        set_block_footer(payload_addr, pack(size + pre_block_size, 0));
        // no need delete or insert
        return pre_block_payload_addr;
    }
    else if (next_alloc == 0 && pre_alloc == 1)
    {
        printf("next merge\n");
        // next merge
        set_block_header(payload_addr, pack(size + next_block_size, 0));
        set_block_header(next_block_payload_addr, pack(size + next_block_size, 0));

        // 删除 next ，插入当前
        delete_block_from_free_list(next_block_payload_addr);
        insert_block_from_free_list(payload_addr);
        return payload_addr;
    }
    else
    {
        printf("merge pre and next\n");
        // merge pre and next
        set_block_header(pre_block_payload_addr, pack(size + pre_block_size + next_block_size, 0));
        set_block_header(next_block_payload_addr, pack(size + pre_block_size + next_block_size, 0));

        // pre 不变，删除 next
        delete_block_from_free_list(next_block_payload_addr);
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

    printf("block_size = %d, align_size = %d\n", block_size, align_size);

    if (block_size > align_size)
    {
        set_block_header(payload_addr, pack(align_size, 1));
        set_block_footer(payload_addr, pack(align_size, 1));

        uint32_t split_block = next_block_payload(payload_addr);
        set_block_header(split_block, pack(block_size - align_size, 0));
        set_block_footer(split_block, pack(block_size - align_size, 0));

        delete_block_from_free_list(payload_addr);
        insert_block_from_free_list(split_block);
        return payload_addr;
    }
    else if (block_size == align_size)
    {
        set_block_header(payload_addr, pack(align_size, 1));
        set_block_footer(payload_addr, pack(align_size, 1));

        delete_block_from_free_list(payload_addr);
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
    set_block_header(heap_listp + WSIZE * 4, pack(0, 1));

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

    uint32_t next_block;
    uint32_t next_block_size;

    for (size_t i = 0; i < free_list_counter; i++)
    {
        next_block = *(uint32_t*)&heap[get_block_next_ptr(free_list_header_payload)];
        next_block_size = get_block_size(next_block);

        // 可以分配这个空闲块
        if (get_block_alloc(next_block) == 0 && next_block_size > align_size)
        {
            printf("try malloc!!!\n");
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