#include "headers/address.h"
#include <stdint.h>
#include <stdio.h>

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

uint8_t sram_cache_read(uint64_t paddr_value)
{
    address_t paddr = {
        .paddr_value = paddr_value,
    };

    sram_cacheset_t *set = &cache.sets[paddr.CI];

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
            invalid =line;
        }
    }

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i)
    {
        sram_cacheline_t *line = &(set->lines[i]);

        if (line->state != CACHE_LINE_INVALID && line->tag == paddr.CT)
        {
            // cache hit
            // update LRU time
            line->time = 0;

            // find the byte
            return line->block[paddr.CO];
        }
    }

    // cache missed
    if (invalid != NULL)
    {
        bus_read_cacheline(paddr.paddr_value, &(invalid->block));

        invalid->state = CACHE_LINE_CLEAN;
        invalid->time = 0;
        invalid->tag = paddr.CT;
        return invalid->block[paddr.CO];
    }

    // need write back first
    if (victim->state == CACHE_LINE_DIRTY)
    {
        bus_write_cacheline(paddr.paddr_value, victim);
    }

    // need replacement
    bus_read_cacheline(paddr.paddr_value, &(victim->block));
    victim->state = CACHE_LINE_CLEAN;
    victim->time = 0;
    victim->tag = paddr.CT;
    return victim->block[paddr.CO];
}

void sram_cache_write(uint64_t paddr_value, uint8_t data)
{
    address_t paddr = {
        .paddr_value = paddr_value,
    };

    sram_cacheset_t *set = &(cache.sets[paddr.CI]);

    int max_time = -1;
    sram_cacheline_t *victim = NULL;
    sram_cacheline_t *invalid = NULL;

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++i)
    {
        sram_cacheline_t *line = &(set->lines[i]);
        line->time ++;

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

        if (line->state != CACHE_LINE_INVALID && line->tag == paddr.CT)
        {
            line->time = 0;
            line->state = CACHE_LINE_DIRTY;
            line->block[paddr.CO] = data;
        }
    }

    // cache missed
    if (invalid != NULL)
    {
        bus_read_cacheline(paddr.paddr_value, &(invalid->block));

        invalid->state = CACHE_LINE_DIRTY;
        invalid->time = 0;
        invalid->tag = paddr.CT;
        invalid->block[paddr.CO] = data;
    }

    // need write back first
    if (victim->state == CACHE_LINE_DIRTY)
    {
        bus_write_cacheline(paddr.paddr_value, victim);
    }

    // need replacement
    bus_read_cacheline(paddr.paddr_value, &(victim->block));
    victim->state = CACHE_LINE_CLEAN;
    victim->time = 0;
    victim->tag = paddr.CT;
    victim->block[paddr.CO] = data;

    return;
}