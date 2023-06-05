#ifndef CACHE_TEST_HEADER
#define CACHE_TEST_HEADER

#include <stdlib.h>
#include <string.h>

typedef struct
{
    int open;
    int cache_hit_count;
    int cache_miss_count;
    int cache_evict_count;
    int dirty_bytes_in_cache_count;
    int dirty_bytes_evicted_count;

    char trace_buf[30];
    char *trace_ptr;

} cache_test_tool_t;

cache_test_tool_t cache_test_tool;

void increase_hit_count();

void increase_miss_count();

void increase_evicted_count();

void fill_trace_buf(char *result);

void reset_buf();

// not safe
void append_buf(char *ch);

#endif