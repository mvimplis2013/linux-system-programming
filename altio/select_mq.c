/*  select_mq.c

    Usage: select_mq msqid ....

    Demonstrate how we can use a child process in conjuction with 
    select() in order to wait for input on a file descriptor (in 
    this case is the terminal) and on a message queue.

    This program allows us to monitor multiple message queues by
    creating a separate child for each queue named on its command
    line .

    For experimenting, you may find it useful to use the msg_create
    and msg_send.c programs from the System V IPC chapter.
*/

#include <sys/time.h>
#include <sys/select.h>
#include <sys/msg.h>
#include <signal.h>
#include <stddef.h>

#include "tlpi_hdr.h"

#define BUF_SIZE 200

/*  Total size of the 'pbuf' struct must not exceed PIPE_BUF, 
    otherwise writes by multiple children may not be atomic,
    with the result that message are broken up and interleaved. */

#define MAX_MTEXT 400

strurt pbuf {
    int msqid;                  /* Origin of message */
    int len;                    /* Number of bytes used in context */
    long mtype;                 /* Message type */
    char mtext[MAX_MTEXT];      /* Message Body */
};

int 
main(int argc, char *argv[]) 
{
    fd_set readfds;
    int ready, nfds, j;
}     



