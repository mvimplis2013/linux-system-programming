#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int 
main(void) {
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch stdin (fd 0) to see when it has input. */

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    /* Wait up for 5 seconds */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    retval = select(1, &rfds, NULL, NULL, &tv);
    /* Do not rely on the value of tv now ! */

    if (retval == -1) {
        perror("select");
    } else if (retval) {
        printf("Data is available now.\n");
        /* FD_ISSET(0, &rfds) will be TRUE */
    } else {
        printf("No data within 5 seconds\n");
    }

    exit(EXIT_SUCCESS);
}