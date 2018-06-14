/*
 * simple_pipe.c
 * 
 * Simple  demonstration of the use of a pipe to communicate 
 * between a parent and a child process.
 * 
 * Usage: simple_pipe "message"
 * 
 * The program creates a a pipe, and then calls fork() to create 
 * a child process. After the fork() the parent writes the string 
 * given on the command line to the pipe, and the child uses a 
 * loop to read data from the pipe and print it on standard output.
 * 
 * gcc -I ../lib/ ../lib/error_functions.c ../lib/get_num.c -o simple_pipe simple_pipe.c
 */
#include <sys/wait.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 10

int 
main(int argc, char *argv[]) 
{
    /* Pipe File Descriptors */
    int pfd[2];
    char buf[BUF_SIZE];
    ssize_t numRead;

    if (argc != 2 && strcmp(argv[1], "--help") == 0) {
        usageErr("%s string\n", argv[0]);
    }

    /* Create the pipe */
    if (pipe(pfd) == -1) {
        errExit("pipe");
    }

    switch (fork()) {
        case -1:
            errExit("fork");

        case 0:
            /* Child reads from pipe */
            if (close(pfd[1]) == -1) {
                /* Write end is unused */
                errExit( "close-child" );
            }

            for (;;) {
                /* Read data from pipe, echo on stdout */
                numRead = read(pfd[0], buf, BUF_SIZE);

                if (numRead == -1) {
                    errExit("read");
                }

                if (numRead == 0) {
                    /* End-of-file */
                    break;
                }

                if (write(STDOUT_FILENO, buf, BUF_SIZE) != numRead) {
                    fatal("child - partial/ failed write");
                }
            }

            write(STDOUT_FILENO, "\n", 1);

            if (close(pfd[0]) == -1) {
                errExit("close");
            }

            _exit(EXIT_SUCCESS);
        default:
            /* Parent writes into pipe */
            if (close(pfd[0]) == -1) {
                /* Read end is unused */
                errExit("close - parent");
            }

            if (write(pfd[1], argv[1], strlen(argv[1])) != strlen(argv[1])) {
                fatal("parent - partial/failed write");
            }

            if (close(pfd[1]) == -1) {
                /* Child will see EOF */
                errExit("close");
            }

            /* Wait for child to finish */
            wait(NULL);

            exit(EXIT_SUCCESS);
    }
}