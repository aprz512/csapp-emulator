#include <stdio.h>

long add(long a, long b)
{
    return a + b;
}

int main()
{

    long x = 0x1234;
    long y = 0xabcd0000;
    long sum = add(x, y);

    printf("sum = 0x%x\n", sum);
    return 0;
}