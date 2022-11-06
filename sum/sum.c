#include <stdio.h>

long sum(long n)
{
    if (n == 1)
    {
        return 1;
    }
    
    return n + sum(n - 1);
}

int main() {

    printf("sum =  %ld\n", sum(5));

    return 0;
}