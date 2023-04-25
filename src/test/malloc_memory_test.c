#include <stdlib.h>

int main()
{
    int *pi = malloc(sizeof(int) * 10);
    pi[0] = 0xa;
    pi[1] = 0xb;
    pi[8] = 0xc;
    pi[9] = 0xd;
    free(pi);
    return 0;
}