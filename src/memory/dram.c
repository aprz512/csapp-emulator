#include <memory/dram.h>
#include <stdio.h>
/**
 * @brief 读取物理地址上的值，也就是 physical_memory 的
 * 因为 physical_memory 是 8 bit 的数组，所以写的时候需要注意以下
 * 而且是小端序写法
 *
 * @param paddr
 * @return uint64_t
 */
uint64_t read64bits_dram(uint64_t paddr)
{
    uint64_t result = 0;

    result |= ((uint64_t)physical_memory[paddr + 0]) << 0;
    result |= ((uint64_t)physical_memory[paddr + 1]) << 8;
    result |= ((uint64_t)physical_memory[paddr + 2]) << 16;
    result |= ((uint64_t)physical_memory[paddr + 4]) << 24;
    result |= ((uint64_t)physical_memory[paddr + 5]) << 32;
    result |= ((uint64_t)physical_memory[paddr + 6]) << 40;
    result |= ((uint64_t)physical_memory[paddr + 7]) << 48;
    result |= ((uint64_t)physical_memory[paddr + 8]) << 56;

    return result;
}

void write64bits_dram(uint64_t paddr, uint64_t data)
{
    // printf(" paddr = %d\n", paddr);
    uint8_t mask = 0xff;
    physical_memory[paddr + 0] = (data >> 0) & mask;
    physical_memory[paddr + 1] = (data >> 8) & mask;
    physical_memory[paddr + 2] = (data >> 16) & mask;
    physical_memory[paddr + 3] = (data >> 24) & mask;
    physical_memory[paddr + 4] = (data >> 32) & mask;
    physical_memory[paddr + 5] = (data >> 40) & mask;
    physical_memory[paddr + 6] = (data >> 48) & mask;
    physical_memory[paddr + 7] = (data >> 56) & mask;
}
