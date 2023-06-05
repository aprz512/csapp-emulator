#include <stdint.h>
#include <stdio.h>
#include "headers/address.h"
#include "headers/memory.h"
#include "headers/cachetest.h"
#include "headers/common.h"

#define NUM_CACHE_LINE_PER_SET (8)

typedef enum
{
    CACHE_LINE_INVALID,
    CACHE_LINE_CLEAN,
    CACHE_LINE_DIRTY
} sram_cacheline_state_t;

typedef struct
{
    sram_cacheline_state_t state;
    int time; // timer to find LRU line inside one set
    uint64_t tag;
    // 每行有64个byte的数据
    uint8_t block[(1 << SRAM_CACHE_OFFSET_LENGTH)];

} sram_cacheline_t;

typedef struct
{
    sram_cacheline_t lines[NUM_CACHE_LINE_PER_SET];
} sram_cacheset_t;

typedef struct
{
    sram_cacheset_t sets[(1 << SRAM_CACHE_INDEX_LENGTH)];
} sram_cache_t;

static sram_cache_t cache;

void sram_reset()
{
    for (size_t i = 0; i < (1 << SRAM_CACHE_INDEX_LENGTH); i++)
    {
        for (size_t j = 0; j < NUM_CACHE_LINE_PER_SET; j++)
        {
            for (size_t k = 0; k < (1 << SRAM_CACHE_OFFSET_LENGTH); k++)
            {
                cache.sets[i].lines[k].state = CACHE_LINE_INVALID;
            }
        }
    }
}

uint8_t sram_cache_read(uint64_t paddr_value)
{
    address_t paddr = {
        .paddr_value = paddr_value,
    };

    sram_cacheset_t *set = &cache.sets[paddr.ci];

    int max_time = -1;
    sram_cacheline_t *victim = NULL;
    sram_cacheline_t *invalid = NULL;

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i)
    {
        sram_cacheline_t *line = &(set->lines[i]);
        line->time++;

        if (max_time < line->time)
        {
            victim = line;
            max_time = line->time;
        }

        if (line->state == CACHE_LINE_INVALID && invalid == NULL)
        {
            invalid = line;
        }
    }

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i)
    {
        sram_cacheline_t *line = &(set->lines[i]);

        if (line->state != CACHE_LINE_INVALID && line->tag == paddr.ct)
        {
            // cache hit
            // update LRU time
            line->time = 0;

            // find the byte
            increase_hit_count();
            append_buf("hit");
            return line->block[paddr.co];
        }
    }

    // cache missed
    if (invalid != NULL)
    {
        increase_miss_count();
        append_buf("miss");
        bus_read_cacheline(paddr.paddr_value, (uint8_t *)&(invalid->block));

        invalid->state = CACHE_LINE_CLEAN;
        invalid->time = 0;
        invalid->tag = paddr.ct;
        return invalid->block[paddr.co];
    }

    increase_hit_count();
    append_buf("eviction");

    // need write back first
    if (victim->state == CACHE_LINE_DIRTY)
    {
        bus_write_cacheline(paddr.paddr_value, (uint8_t *)&(victim->block));
    }

    // need replacement
    bus_read_cacheline(paddr.paddr_value, (uint8_t *)&(victim->block));
    victim->state = CACHE_LINE_CLEAN;
    victim->time = 0;
    victim->tag = paddr.ct;
    return victim->block[paddr.co];
}

void sram_cache_write(uint64_t paddr_value, uint8_t data)
{
    address_t paddr = {
        .paddr_value = paddr_value,
    };

    sram_cacheset_t *set = &(cache.sets[paddr.ci]);

    int max_time = -1;
    sram_cacheline_t *victim = NULL;
    sram_cacheline_t *invalid = NULL;

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i)
    {
        sram_cacheline_t *line = &(set->lines[i]);
        line->time++;

        if (max_time < line->time)
        {
            victim = line;
            max_time = line->time;
        }

        if (line->state == CACHE_LINE_INVALID && invalid == NULL)
        {
            invalid = line;
        }
    }

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i)
    {
        sram_cacheline_t *line = &(set->lines[i]);

        if (line->state != CACHE_LINE_INVALID && line->tag == paddr.ct)
        {
            line->time = 0;
            line->state = CACHE_LINE_DIRTY;
            line->block[paddr.co] = data;
            increase_hit_count();
            append_buf("hit");
            return;
        }
    }

    // cache missed
    if (invalid != NULL)
    {
        // my_log(DEBUG_CACHE, "invalid block address=%p\n", &(invalid->block));
        bus_read_cacheline(paddr.paddr_value, (uint8_t *)&(invalid->block));

        invalid->state = CACHE_LINE_DIRTY;
        invalid->time = 0;
        invalid->tag = paddr.ct;
        invalid->block[paddr.co] = data;

        increase_miss_count();
        append_buf("miss");
        return;
    }

    increase_evicted_count();
    append_buf("eviction");

    // need write back first
    if (victim->state == CACHE_LINE_DIRTY)
    {
        bus_write_cacheline(paddr.paddr_value, (uint8_t *)&(victim->block));
        victim->state = CACHE_LINE_INVALID;
    }

    // write-allocate
    bus_read_cacheline(paddr.paddr_value, (uint8_t *)&(victim->block));
    victim->state = CACHE_LINE_DIRTY;
    victim->time = 0;
    victim->tag = paddr.ct;
    victim->block[paddr.co] = data;

    return;
}

void increase_hit_count()
{
    if (!cache_test_tool.open)
    {
        return;
    }
    cache_test_tool.cache_hit_count++;
}

// void decrease_hit_count() {
//     cache_test_tool.cache_hit_count--;
// }

void increase_miss_count()
{
    if (!cache_test_tool.open)
    {
        return;
    }
    cache_test_tool.cache_miss_count++;
}

// void decrease_miss_count() {
//     cache_test_tool.cache_hit_count--;
// }

void increase_evicted_count()
{
    if (!cache_test_tool.open)
    {
        return;
    }
    cache_test_tool.cache_evict_count++;
}

// void decrease_evicted_count() {
//     cache_test_tool.cache_hit_count--;
// }

void fill_trace_buf(char *result)
{
    if (!cache_test_tool.open)
    {
        return;
    }
    int i = 0;
    while (*result != '\0' && i < 30)
    {
        cache_test_tool.trace_buf[i] = *(result + i);
        i++;
    }
}

void reset_buf()
{
    if (!cache_test_tool.open)
    {
        return;
    }
    for (size_t i = 0; i < 30; i++)
    {
        cache_test_tool.trace_buf[i] = '\0';
    }
}

// not safe
void append_buf(char *ch)
{
    if (!cache_test_tool.open)
    {
        return;
    }

    int append_len = strlen(ch);
    int current_len = strlen(cache_test_tool.trace_buf);

    if (current_len != 0)
    {
        cache_test_tool.trace_buf[current_len] = ' ';
        cache_test_tool.trace_buf[current_len + 1] = '\0';
    }

    current_len = strlen(cache_test_tool.trace_buf);

    for (size_t i = 0; i < append_len; i++)
    {
        cache_test_tool.trace_buf[current_len + i] = ch[i];
    }
    cache_test_tool.trace_buf[current_len + append_len + 1] = '\0';
    my_log(DEBUG_CACHE, "append buf = |%s|\n", cache_test_tool.trace_buf);
}

void trim(char *ch)
{
    // my_log(DEBUG_CACHE, "before trim, line = %s\n", ch);
    int len = strlen(ch);
    while (ch[0] == ' ')
    {
        for (size_t i = 0; i < len; i++)
        {
            ch[i] = ch[i + 1];
        }
    }

    int new_len = strlen(ch);
    int index = 0;
    for (size_t i = new_len - 1; i >= 0; i--)
    {
        if (ch[i] == '\n' || ch[i] == ' ')
        {
            ch[i] = '\0';
        }
        else
        {
            break;
        }
    }
    // my_log(DEBUG_CACHE, "after trim, line[0] = %c\n", ch[0]);
}

uint64_t read_addr(char *line)
{
    int len = strlen(line);
    int end = 0;
    for (size_t i = 2; i < len; i++)
    {
        if (line[i] == ',')
        {
            end = i;
            break;
        }
    }

    char address[20];
    address[0] = '0';
    address[1] = 'x';
    for (size_t i = 2; i < 20; i++)
    {
        address[i] = line[i];
    }

    return string2uint_range(address, 0, end - 1);
}

void check_counter(char *line)
{
    int len = strlen(line);
    int start = 0;
    int count = 0;
    int hits = 0;
    int misses = 0;
    int evictions = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (line[i] == ':')
        {
            start = i;
        }

        if (line[i] == ' ')
        {
            if (count == 0)
            {
                hits = string2uint_range(line, start + 1, i - 1);
                count++;
            }
            else if (count == 1)
            {
                misses = string2uint_range(line, start + 1, i - 1);
                count++;
            }
        }
    }

    evictions = string2uint_range(line, start + 1, -1);

    if (hits == cache_test_tool.cache_hit_count && misses == cache_test_tool.cache_miss_count && evictions == cache_test_tool.cache_evict_count)
    {
        printf("total pass!!!\n");
    }
    else
    {
        printf("failed!!! hit count = %d, miss count = %d, eviction count = %d\n", hits, misses, evictions);
    }
}