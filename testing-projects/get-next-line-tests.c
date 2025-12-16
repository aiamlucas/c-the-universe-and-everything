#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "get_next_line.h"

/*
    This main uses ONE file only: test.txt

    For each test:
    - Edit test.txt according to the comment
    - Comment / uncomment tests if needed
    - Compile and run
*/

int main(void)
{
    int     fd;
    char    *line;

    /* ===================================================== */
    /* TEST 1: Simple lines                                  */
    /* test.txt content should be:                           */
    /* Hello                                                  */
    /* 42                                                     */
    /* Network                                                */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 1: simple lines\n");
    line = get_next_line(fd);
    printf("1 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("2 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("3 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("4 -> [%s]\n", line);
    close(fd);

    /* ===================================================== */
    /* TEST 2: Empty file                                    */
    /* test.txt content should be:                           */
    /* (empty file)                                          */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 2: empty file\n");
    line = get_next_line(fd);
    printf("1 -> [%s]\n", line);
    close(fd);

    /* ===================================================== */
    /* TEST 3: Only newline                                  */
    /* test.txt content should be:                           */
    /* (one empty line, just a newline)                      */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 3: only newline\n");
    line = get_next_line(fd);
    printf("1 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("2 -> [%s]\n", line);
    close(fd);

    /* ===================================================== */
    /* TEST 4: No newline at EOF                             */
    /* test.txt content should be:                           */
    /* This file has no newline at the end                   */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 4: no newline at EOF\n");
    line = get_next_line(fd);
    printf("1 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("2 -> [%s]\n", line);
    close(fd);

    /* ===================================================== */
    /* TEST 5: Multiple consecutive newlines                 */
    /* test.txt content should be:                           */
    /*                                                       */
    /*                                                       */
    /* Hello                                                 */
    /*                                                       */
    /*                                                       */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 5: multiple newlines\n");
    line = get_next_line(fd);
    printf("1 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("2 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("3 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("4 -> [%s]\n", line);
    close(fd);

    /* ===================================================== */
    /* TEST 6: Long line (BUFFER_SIZE stress)                */
    /* test.txt content should be:                           */
    /* Lorem ipsum dolor sit amet, consectetur adipiscing... */
    /* ===================================================== */
    fd = open("test.txt", O_RDONLY);
    printf("\nTEST 6: long line\n");
    line = get_next_line(fd);
    printf("1 -> [%s]\n", line);
    free(line);
    line = get_next_line(fd);
    printf("2 -> [%s]\n", line);
    close(fd);

    return (0);
}
