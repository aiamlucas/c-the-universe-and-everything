#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "get_next_line_bonus.h"

/*
    SIMPLE BONUS TEST
    Two file descriptors, interleaved reads.
*/

int main(void)
{
    int     fd1;
    int     fd2;

    fd1 = open("file1.txt", O_RDONLY);
    fd2 = open("file2.txt", O_RDONLY);

    printf("fd1 -> %s\n", get_next_line(fd1)); // A1\n
    printf("fd2 -> %s\n", get_next_line(fd2)); // B1\n
    printf("fd1 -> %s\n", get_next_line(fd1)); // A2\n
    printf("fd2 -> %s\n", get_next_line(fd2)); // B2\n

    close(fd1);
    close(fd2);

    return (0);
}
