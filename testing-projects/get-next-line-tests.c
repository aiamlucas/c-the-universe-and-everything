#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "get_next_line.h"

/*
    ONE file: test.txt
    Change its content between runs
*/

int main(void)
{
    int     fd;
    char    *line;
    int     i;

    /* ===================================================== */
    /* TEST 1: Simple lines                                  */
    /* test.txt content:                                     */
    /* Hello                                                  */
    /* 42                                                     */
    /* Network                                                */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 1: simple lines\n");
    i = 1;
    while ((line = get_next_line(fd)))
    {
        printf("%d -> [%s]\n", i++, line);
        free(line);
    }
    printf("%d -> [NULL]\n", i);
    close(fd);

    /* ===================================================== */
    /* TEST 2: Empty file                                    */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 2: empty file\n");
    line = get_next_line(fd);
    printf("1 -> [%s]\n", line);
    close(fd);

    /* ===================================================== */
    /* TEST 3: Only newline                                  */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 3: only newline\n");
    i = 1;
    while ((line = get_next_line(fd)))
    {
        printf("%d -> [%s]\n", i++, line);
        free(line);
    }
    printf("%d -> [NULL]\n", i);
    close(fd);

    /* ===================================================== */
    /* TEST 4: No newline at EOF                             */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 4: no newline at EOF\n");
    i = 1;
    while ((line = get_next_line(fd)))
    {
        printf("%d -> [%s]\n", i++, line);
        free(line);
    }
    printf("%d -> [NULL]\n", i);
    close(fd);

    /* ===================================================== */
    /* TEST 5: Multiple newlines                             */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 5: multiple newlines\n");
    i = 1;
    while ((line = get_next_line(fd)))
    {
        printf("%d -> [%s]\n", i++, line);
        free(line);
    }
    printf("%d -> [NULL]\n", i);
    close(fd);

    /* ===================================================== */
    /* TEST 6: Long line                                     */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 6: long line\n");
    i = 1;
    while ((line = get_next_line(fd)))
    {
        printf("%d -> [%s]\n", i++, line);
        free(line);
    }
    printf("%d -> [NULL]\n", i);
    close(fd);

    return (0);
}
