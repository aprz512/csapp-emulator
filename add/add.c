#include <stdio.h>

long add(long a, long b)
{
    long ret = a + b;
    return ret;
}

int main()
{

    long x = 0x1234;
    long y = 0xabcd0000;
    long sum = add(x, y);

    printf("sum = 0x%lx\n", sum);
    return 0;
}