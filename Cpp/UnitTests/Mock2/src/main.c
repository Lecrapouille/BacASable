#include "example.h"

// Standalone example.
//   gcc -W -Wall example.c main.c -o example
// Note that this main is not important since
// the goal of this mini-project is to show
// how to unit test the function example() which
// uses functions such as open() and read().
int main()
{
    char buffer[32u];

    int fd = example(buffer, 32u);
    close(fd);

    return 0;
}
