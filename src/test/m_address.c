#include <stdint.h>
#include <stdio.h>

typedef union
{
    uint64_t address;
    struct
    {
        uint8_t d0;
        uint8_t d1;
        uint8_t d2;
        uint8_t d3;
        uint8_t d4;
        uint8_t d5;
        uint8_t d6;
        uint8_t d7;
    };
} m_address_t;

int main()
{

    int a = 0;

    m_address_t ma = {
        .address = &a
    };

    printf("address = %p\nd0=%x\nd1=%x\nd2=%x\nd3=%x\nd4=%x\nd5=%x\nd6=%x\nd7=%x\n", ma.address, ma.d0, ma.d1, ma.d2, ma.d3, ma.d4, ma.d5, ma.d6, ma.d7);

    return 0;
}
