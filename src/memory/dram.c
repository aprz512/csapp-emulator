#include "headers/memory.h"
#include "headers/common.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "headers/address.h"

uint8_t sram_cache_read(uint64_t paddr);
void sram_cache_write(uint64_t paddr, uint8_t data);

/**
 * @brief 读取物理地址上的值，也就是 physical_memory 的
 * 因为 physical_memory 是 8 bit 的数组，所以写的时候需要注意以下
 * 而且是小端序写法
 *
 * @param paddr
 * @return uint64_t
 */
// memory accessing used in instructions
uint64_t cpu_read64bits_dram(uint64_t paddr)
{
    if (DEBUG_ENABLE_SRAM_CACHE == 1)
    {
        uint64_t val = 0x0;
        for (int i = 0; i < 8; ++i)
        {
            val += ((uint64_t)(sram_cache_read(paddr + i)) << (i * 8));
        }
        return val;
    }
    else
    {
        // read from DRAM directly
        // little-endian
        uint64_t val = 0x0;

        val += (((uint64_t)pm[paddr + 0]) << 0);
        val += (((uint64_t)pm[paddr + 1]) << 8);
        val += (((uint64_t)pm[paddr + 2]) << 16);
        val += (((uint64_t)pm[paddr + 3]) << 24);
        val += (((uint64_t)pm[paddr + 4]) << 32);
        val += (((uint64_t)pm[paddr + 5]) << 40);
        val += (((uint64_t)pm[paddr + 6]) << 48);
        val += (((uint64_t)pm[paddr + 7]) << 56);

        return val;
    }
}

void cpu_write64bits_dram(uint64_t paddr, uint64_t data)
{
    if (DEBUG_ENABLE_SRAM_CACHE == 1)
    {
        for (int i = 0; i < 8; ++i)
        {
            sram_cache_write(paddr + i, (data >> (i * 8)) & 0xff);
        }
        return;
    }
    else
    {
        // write to DRAM directly
        // little-endian
        pm[paddr + 0] = (data >> 0) & 0xff;
        pm[paddr + 1] = (data >> 8) & 0xff;
        pm[paddr + 2] = (data >> 16) & 0xff;
        pm[paddr + 3] = (data >> 24) & 0xff;
        pm[paddr + 4] = (data >> 32) & 0xff;
        pm[paddr + 5] = (data >> 40) & 0xff;
        pm[paddr + 6] = (data >> 48) & 0xff;
        pm[paddr + 7] = (data >> 56) & 0xff;
    }
}

/**
 * @brief 读取内存中的指令
 *
 * @param paddr 内存指令地址
 * @param buf 指令要存放空间
 */
void cpu_readinst_dram(uint64_t paddr, char *buf)
{
    for (size_t i = 0; i < MAX_INSTRUCTION_CHAR; i++)
    {
        buf[i] = pm[paddr + i];
    }
    my_log(DEBUG_MMU, "read inst: %s\n", buf);
}

/**
 * @brief 将字符串指令写入到内存中，模拟程序加载到内存的过程
 *
 * @param paddr 写入地址
 * @param str 字符串指令
 */
void cpu_writeinst_dram(uint64_t paddr, const char *str)
{
    int len = strlen(str);
    assert(len < MAX_INSTRUCTION_CHAR);
    for (size_t i = 0; i < MAX_INSTRUCTION_CHAR; i++)
    {
        pm[paddr + i] = i < len ? str[i] : 0;
    }

    my_log(DEBUG_MMU, "write inst: %s, inst len = %d\n", str, len);
}

// 从磁盘中读取数据到缓存行的一页数据（在该页物理地址的范围内的都可以）
void bus_read_cacheline(uint64_t paddr, uint8_t *block)
{
#ifndef CACHE_TEST
    // my_log(DEBUG_CACHE, "block address=%p\n", block);
    uint64_t dram_base = ((paddr >> SRAM_CACHE_OFFSET_LENGTH) << SRAM_CACHE_OFFSET_LENGTH);

    for (int i = 0; i < (1 << SRAM_CACHE_OFFSET_LENGTH); ++i)
    {
        // my_log(DEBUG_CACHE, "block address=%p, i = %d\n", block, i);
        block[i] = pm[dram_base + i];
    }
#endif
}

void bus_write_cacheline(uint64_t paddr, uint8_t *block)
{
#ifndef CACHE_TEST
    uint64_t dram_base = ((paddr >> SRAM_CACHE_OFFSET_LENGTH) << SRAM_CACHE_OFFSET_LENGTH);

    for (int i = 0; i < (1 << SRAM_CACHE_OFFSET_LENGTH); ++i)
    {
        pm[dram_base + i] = block[i];
    }
#endif
}