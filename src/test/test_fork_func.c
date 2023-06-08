#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

int main()
{
    pid_t pid;
    int x = 1;

    pid = fork();

    if (pid == 0)
    {
        /* Child */
        printf("child : x=%d, pid = %d\n", ++x, getpid());
        while (1)
        {
        }
        return 0;
    }

    /* Parent */
    printf("parent: x=%d, pid = %d\n", --x, getpid());
    while (1)
    {
    }
    return 0;
}