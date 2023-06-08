#include <stdio.h>
#include <stdlib.h>

int main()
{
    // 分配 10 个 byte
    char * buf = malloc(10);
    buf[10] = '9';
    printf("ok\n");
    buf[135168] = '9';
    printf("ok\n");
    while (1)
    {
        /* code */
    }
    
    return 0;
}