#include <stdio.h>
#include <stdint.h>

int main() 
{
    uint8_t x = 0xFF;
    uint8_t y = 0xFE;
    printf("r = %u\n", (y-x));
    return 0;
}