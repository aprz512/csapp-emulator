#include <stdio.h>


int bias = 0x55;
int array[2] = {0xaa, 0xbb};

int main() {
    
    printf("%d-%d-%d\n", bias, array[0], array[1]);
    return 0;
}