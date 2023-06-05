#include <stdio.h>

int main() {

    int a[] = {0x12345678, 0x12345678};

    printf("a_addr = %p\n",  &a[0]);
    printf("a_addr = %p\n",  &a[1]);

    int b[2] = {0x12345678, 0x12345678};

    // printf("a_addr = %p\n",  &a[0]);
    // printf("a_addr = %p\n",  &a[1]);
    printf("b_addr = %p\n",  &b[0]);
    printf("b_addr = %p\n",  &b[1]);

    return 0;
}