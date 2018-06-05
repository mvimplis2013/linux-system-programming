/*
 * svmsg_send.c
 * 
 * Usage: svmsg_send [-n] msqid msg-type [msg-text]
 * 
 * Experiment with the msgsdn() system call to send messages to a 
 * System V message queue.
 * 
 * See also svmsg_receive.c
 * 
 * gcc -I ../lib/ ../lib/error_functions.c ../lib/get_num.c ../svmsg/svmsg_send.c
 */
#include <sys/types.h>
#include <sys/msg.h>

#include "tlpi_hdr.h"

#define MAX_MTEXT 1024

struct mbuf {
    long mtype;                 /* Message Type */
    char mtext[MAX_MTEXT];      /* Message Body */
};

static void 
usageError(const char *progName, const char *msg)
{
    /* Print (optional) message, then usage description */
    if (msg != NULL) {
        fprintf(stderr, "%s", msg);
    }
    fprintf(stderr, "Usage: %s [-n] msqid msg-type [msg-text]\n", progName);
    fprintf(stderr, "   -n      Use IPC_NOWAIT flag\n");

    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) 
{
    int msqid, flags, msgLen;
    struct mbuf msg;                /* Message bufefr fro msgsnd() */
    int opt;                        /* Option character from getopt() */

    /* Parse command-line options and arguments */
    flags = 0;
    while((opt=getopt(argc, argv, "n")) != -1) {
        if (opt == 'n')
            flags |= IPC_NOWAIT;
        else 
            usageError(argv[0], NULL);
    }

    if (argc < optind + 2 || argc > optind + 3) {
        usageError(argv[0], "Wromgh number of arguments\n");
    }

    msqid = getInt(argv[optind], 0, "msqid");
    msg.mtype = getInt(argv[optind+1], 0, "msg-type");

    if (argc > optind + 2) {
        /* Message text was supplied */
        msgLen = strlen(argv[optind+2]) + 1;

        if (msgLen > MAX_MTEXT) {
            cmdLineErr("msg-text too long (max: %d characters)\n", MAX_MTEXT);
        }

        memcpy(msg.mtext, argv[optind+2], msgLen);        
    }
    else {
        /* No Message text => Zero length message */
        msgLen = 0;
    }

    /* Send message */
    if (msgsnd(msqid, &msg, msgLen, flags) == -1) {
        errExit("msgsnd");
    }

    exit(EXIT_SUCCESS);
}