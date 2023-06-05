#include "headers/allocator.h"
#include "headers/algorithm.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

uint8_t heap[MAX_HEAP_SIZE];
uint64_t prologue_payload;
uint64_t epilogue_payload; // epilogue 没有 payload，让它指向 header 的最后
uint64_t mem_brk;

void check_heap_correctness()
{
    int linear_free_counter = 0;
    uint64_t block_payload = get_first_block_payload();
    while (block_payload <= get_last_block_payload())
    {
        assert(block_payload % 8 == 0);

        uint32_t block_size = get_block_size(block_payload);
        uint32_t header = *(uint32_t *)&heap[block_payload - WSIZE];
        uint32_t footer = *(uint32_t *)&heap[block_payload + block_size - 2 * WSIZE];

        assert(header == footer);

        // 只能有一个连续的 free
        if (get_block_alloc(block_payload) == 0)
        {
            linear_free_counter += 1;
        }
        else
        {
            linear_free_counter = 0;
        }
        assert(linear_free_counter <= 1);

        block_payload = next_block_payload(block_payload);
    }
}

void test_malloc_and_free()
{

    mm_init();
    check_heap_correctness();

    srand(123456);

    linkedlist_t *ptrs = linkedlist_construct();

    for (size_t i = 0; i < 10; i++)
    {
        int r = rand();
        if (r % 2 == 0)
        {
            uint32_t request_size = r % 1024 + 1;
            printf("[%lu] mm_malloc(%u)", i, request_size);

            uint64_t ptr = mm_malloc(request_size);
            if (ptr != 0)
            {
                printf("linkedlist_add\n");
                linkedlist_add(ptrs, ptr);
            }
        }
        else
        {
            printf("ptrs->count = %ld\n", ptrs->count);
            if (ptrs->count != 0)
            {
                int random_index = rand() % ptrs->count;
                linkedlist_node_t *node = linkedlist_index(ptrs, random_index);
                printf("[%ld] mm_free(%lu)", i, node->value);
                mm_free(node->value);
            }
        }
        check_heap_correctness();
        printf("looping... %ld\n", i);
    }
}

int main()
{
    test_malloc_and_free();
    printf("pass!!!\n");

    return 0;
}