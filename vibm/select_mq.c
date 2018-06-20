/*
 * select_mq.c
 *
 * Usage: select_mq  msqid...
 * 
 * Demonstrate how we can use a child process in conjunction with select in order to wait for input on a file descriptor
 * (in this case the terminal) and on a message queue.
 * 
 * This program allows us to monitor multiple message queues by creating a separate child for each queue named on its 
 * command line.
 * 
 * For experimenting, you may find it useful to use the msg_create.c and msg_send.c programs 
 * 
 * gcc -I ../lib/ ../lib/get_num.c ../lib/error_functions.c select_mq.c -o select_mq
 */

#include <sys/time.h>
#include <sys/select.h>
#include <sys/msg.h>
#include <signal.h>
#include <stddef.h>

#include "tlpi_hdr.h"

#define BUF_SIZE 200

/* Total size of the `pbuf` struct must not exceed PIPE_BUF, otherwise writes by multiple children may not be ATOMIC,
 * with the result that messages are broken up and interleaved.
 */

#define MAX_MTEXT 400

struct pbuf {
    int msqid;              /* Origin of message */
    int len;                /* Number of bytes used in mtext */
    long mtype;             /* Message type */
    char mtext[MAX_MTEXT];  /* Message body */
};

/* Function called by child: monitors message queue identified by `msqid`, 
 * copying every message to the pipe identified by `fd`.
 */
static void 
childMon(int msqid, int fd)
{
    struct pbuf pmsg;
    ssize_t msgLen;
    ssize_t wlen;

    for (;;) {
        msgLen = msgrcv(msqid, &pmsg.mtype, )
    }
}
int 
main(int argc, char *argv[]) {
    fd_set readfds;
    int ready, nfds, j;
    int pfd[2];             /* Pipe used to transfer messages from children to parent */

    ssize_t numRead;
    char buf[BUF_SIZE];
    struct pbuf pmsg;

    if (argc < 2 || strcmp(argv[1], "--help") == 0 ) {
        usageErr("%s msqid...\n", argv[0]);
    }

    /* Create the pipe to transfer messages */ 
    if (pipe(pfd) == -1) {
        errExit("pipe");
    }

    /* Create one child for each message queue being monitored */
    for (j=1; j<argc; j++) {
        switch (fork()) {
            case -1:
                errMsg("fork");
                killpg(0, SIGTERM);
                _exit(EXIT_FAILURE);
            case 0:
                childMon(getInt(argv[j], 0, "msqid"), pfd[1]);
                _exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    /* Parent falls through to here */
    for (;;) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(pfd[0], &readfds);

        nfds = max(STDIN_FILENO, pfd[0]) + 1;

        ready = select(nfds, &readfds, NULL, NULL, NULL);

        if (ready == -1) {
            errExit("select");
        }

        // Check if Terminal fd is ready */
        if (FD_ISSET(pfd[0]), &readfds) {
            numRead = read(STDIN_FILNO, buf, BUF_SIZE-1);

            if (numRead == -1) {
                errExit("read stdin");
            }
        }

        buf[numRead] = '\0';
        printf("Read from terminal: %s", buf);
        if (numRead > 0 && buf[numRead-1] != '\n') {
            printf("\n");
        }

        /* Check if pipe fd is ready */
        if (FD_ISSET(pfd[0, &readfds)) {
            numRead = read(pfd[0], &pmsg, offsetofof(struct pbuf, mtext));
        }
    }
}
