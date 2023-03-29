#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "headers/common.h"

void test_unsigned_value()
{
    uint8_t x = 0xFF;
    uint8_t y = 0xFE;
    printf("r = %u\n", (y - x));
}

void test_convert()
{
    uint64_t val = string2uint("0x1234");
    printf("val = %lx, val = %ld\n", val, val);

    uint64_t val2 = string2uint("-0x1234");
    printf("val2 = %lx, val2 = %ld\n", val2, val2);

    uint64_t val3 = string2uint("1234");
    printf("val3 = %lx, val3 = %ld\n", val3, val3);

    uint64_t val4 = string2uint("-1234");
    printf("val4 = %lx, val4 = %ld\n", val4, val4);

    uint64_t val5 = string2uint("0001234");
    printf("val5 = %lx, val5 = %ld\n", val5, val5);

    uint64_t val6 = string2uint("-0001234");
    printf("val6 = %lx, val6 = %ld\n", val6, val6);
}

void TestParseInstruction();
void TestParseOperand();

void AddTestEntry();
void SumTestEntry();

int main()
{
    // AddTestEntry();
    // SumTestEntry();
    return 0;
}