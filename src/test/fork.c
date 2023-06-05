
#include <unistd.h>

int test() {
    return 1;
}

int main() {

    fork();

    test();

    return 0;
}