#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/file.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    fd_set rfds;
    int fd = 0;
    struct timeval timeout;
    int select_res;

    /* Initialize the file descriptor set and add the device file */
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    /* Set the timeout period */
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    /* The device file will block until the DMA transfer has completed */
    select_res = select(FD_SETSIZE, &rfds, NULL, NULL, &timeout);

    /* Reset the channel */
    
    if (select_res == -1) {
        /* Select has encountered an error */
        perror("ERROR <Interrupt Select Failed>\n");
        exit(0);
    }
    else if (select_res == 1) {
        /* The device file descriptor block released */
        return 0;
    }
    else {
        /* The device file descriptor block exceeded timeout */
        return EINTR;
    }
}