#include <stdlib.h>

int main()
{
    char *pc = (char *)malloc(sizeof(char) * 6);
    pc[0] = 'h';
    pc[1] = 'e';
    pc[2] = 'l';
    pc[3] = 'l';
    pc[4] = '0';
    pc[5] = '\0';
    free(pc);
    pc = (char *)malloc(sizeof(char) * 100);

    return 0;
}