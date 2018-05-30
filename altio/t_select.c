/*  t_select.c

    Example of the use of the select() system call to monitor multiple file descriptors.

    Usage as shown in usageError().
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/select.h>

static void
usageError(const char *progName)
{
    fprintf(stderr, "Usage: %s {timeout|-} fd-num[rw]...\n", progName);
    fprintf(stderr, "   - means infinite timeout; \n");
    fprintf(stderr, "   r = monitor for read\n");
    fprintf(stderr, "   w = monitor for write\n\n");
    fprintf(stderr, "   e.g.: %s - 0rw 1w\n", progName);

    exit(EXIT_FAILURE);
}

int 
main(int argc, char *argv[]) {
    fd_set readfds, writefds;
    struct timeval *pto;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageError(argv[0]);

    /* Timeout for select() is specified in argv[1] */
}

