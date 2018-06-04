/* 
 * poll_pipes.c
 * 
 * Example of the use of poll() to monitor multiple file descriptors.
 * 
 * Usage: poll_pipes num-pipes [num-writes]
 * Default: One(1)
 * 
 * Create 'num-pipes' pipes, and perform 'num-writes' writes to
 * randomly selected pipes. Then use poll() to inspect the read
 * ends of the pipes to see which pipes are readable.
 */
#include <time.h>
#include <poll.h>

#include "tlpi_hdr.h"

int main(int argc, char *argv[]) {
    int numPipes, j, ready, randPipe, numWrites;
    int (*pfds)[2];
    struct pollfd *pollFd;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s num-pipes [num-writes]\n", argv[0]);
    }

    /* Allocate the arrays that we use. The arrays are sized 
     * according to the number of pipes specified on command 
     * line.
     */
    numPipes = getLong(argv[1], GN_GT_0, "num-pipes");

    pfds = calloc(numPipes, sizeof(int [2]));
    if (pfds == NULL) {
        errExit("calloc fds");
    }

    pollFd = calloc(numPipes, sizeof(struct pollfd));
    if (pollFd == NULL) {
        errExit("callco pollFds");
    }

    /* Create a number of pipes specified on command line */
    for (j=0; j<numPipes; j++) {
        if (pipe(pfds[j]) == -1) {
            errExit("pipe %d", j);
        }
    }

    /* Perform specied number of writes to random pipes */
    numWrites = (argc > 2) ? getInt(argv[2], GN_GT_0, "num-writes"): 1;

    srandom((int) time(NULL));
    for (j=0; j<numWrites; j++) {
        randPipe = random() % numPipes;
        printf("Writing to fd: %3d (read fd: %3d)\n", 
            pfds[randPipe][1], pfds[randomPipe][0]);

        if (write(pfds[randomPipe][1], "a", 1) == -1) {
            errExit("Write %d", pfds[randPipe][1]);
        }
    }  

    /* Build the file descriptor list to be supplied to poll().
     * This list is set to contain the file descriptors for 
     * the read ends of all the pipes.
     */
    for (j=0; j<numPipes; j++) {
        pollFd[j].fd = pfds[j][0];
        pollFd[j].events = POLLIN;
    }

    ready = poll(pollFd, numPipes, 0);
    if (ready == -1)
        errExit("poll");

    for (j=0; j<numPipes; j++) {
        if (pollFd[j].revents & POLLIN) {
            printf("Readable: %3d\n", polFd[j].fd);
        }
    }

    exit(EXIT_SUCCESS);
}