#include <stdio.h>

int a;

int main()
{
    // int a = 10;
    int b = a * 2;
    int c = a * 4;
    int d = 8 * a;

    int e = a + b + c + d;

    printf("%d", e);

    return 0;
}