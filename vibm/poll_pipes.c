/*
 * poll_pipes.c
 * 
 * Example of the use of poll() to monitor multiple file descriptors.
 * 
 * Usage: poll_pipes num-pipes [num-writes] 
 *                                def = 1
 * 
 * Create 'num-pipes' pipes, and perform 'num-writes' writes to
 * randomly selected pipes. Then use poll() to inspect the 
 * read-ends of the pipes to see which pipes are readable.
 * 
 * gcc -I ../lib/ ../lib/error_functions.c ../lib/get_num.c -o poll_pipes poll_pipes.c
 */

#include <time.h>
#include <poll.h>
#include "tlpi_hdr.h"

int main(int argc, char *argv[]) {
    int numPipes, j, ready, randPipe, numWrites;
    
    /* File descriptor for all pipes */ 
    int (*pfds)[2];

    struct pollfd *pollFd;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s mum-pipes [num-writes]\n", argv[0]);
    }

    /* Allocate the arrays we will use. The arrays are sized 
     * according to the number of pipes specifies on 
     * command line.
     */
    numPipes = getInt(argv[1], GN_GT_0, "num-pipes");

    pfds = calloc(numPipes, sizeof(int [2]));

    if (pfds == NULL) {
        errExit("calloc");
    }

    pollFd = calloc(numPipes, sizeof(struct pollfd));
    if (pollFd == NULL) {
        errExit("calloc");
    }

    /* Create the number of pipes specified on command line */
    for (j=0; j<numPipes; j++) {
        if ( pipe(pfds[j]) == -1 ) {
            errExit("pipe %d", j);
        } 
    }

    /* Perform specified number of writes to random pipes */
    numWrites = (argc > 2) ? getInt(argv[2], GN_GT_0, "num-writes") : 1;

    srandom((int) time(NULL));
    for (j=0; j<numWrites; j++) {
        randPipe = random() % numPipes;
        printf("Writing to fd: %3d (read fd: %3d)\n", 
            pfds[randPipe], pfds[randPipe][0]);

        if (write(pfds[randPipe][1], "a", 1) == -1) {
            errExit("write %d", pfds[randPipe][1]);
        }
    }

    /* Build the file descriptor list to be supplied to poll().
     * This list is set to contain the file descriptor for the 
     * read ends of all of the pipes.
     */
    for (j=0; j<numPipes; j++) {
        pollFd[j].fd = pfds[j][0];
        pollFd[j].events = POLLIN;
    }

    ready = poll(pollFd, numPipes, 0);
    if (ready == -1) {
        errExit("poll");
    }

    printf("poll() returned: %d\n", ready);

    /* Check which pipes have data available for reading */
    for (j=0; j<numPipes; j++) {
        if (pollFd[j].revents & POLLIN) {
            printf("Readable: %3d\n", pollFd[j].fd);
        }
    }

    exit(EXIT_SUCCESS);
}