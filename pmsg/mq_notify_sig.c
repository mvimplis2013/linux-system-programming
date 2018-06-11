/*
 * mq_notify_sig.c
 * 
 * Usage: mq_notify_sig mq-name
 * 
 * Demonstrate message notification via signals (catching the 
 * signals with a signal handler) on a POSIX message queue.
 * 
 * gcc -I ../lib/ ../lib/error_functions.c ../lib/get_num.c -o mq_notify_sig  mq_notify_sig.c -lrt
 */
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>

#include "tlpi_hdr.h"

#define NOTIFY_SIG SIGUSR1

static void
handler(int sig)
{
    /* Just inerrupt sigsuspend() */
} 

int 
main(int argc, char *argv[]) {
    struct sigevent sev;
    mqd_t mqd;
    struct mq_attr attr;
    void *buffer;
    ssize_t numRead;
    sigset_t blockMask, emptyMask;
    struct sigaction sa;

    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s mq-name\n", argv[0]);
    }

    mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK );
    if (mqd == (mqd_t) -1) {
        errExit("mq_open");
    }

    /* 
     * Determine mq_msgsize for message queue and allocate 
     * an input buffer of that size.
     */
    if (mq_getattr(mqd, &attr) == -1) {
        errExit("mq_getattr");
    }

    buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL) {
        errExit("malloc");
    }

    /* Block the notification signal and esatblish 
     * a handler for it.
     */
    sigemptyset(&blockMask);
    sigaddset(&blockMask, NOTIFY_SIG);
    if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1) {
        errExit("sigprocmask");
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(NOTIFY_SIG, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    /* Register for message notification via a signal */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = NOTIFY_SIG;
    if (mq_notify(mqd, &sev) == -1) {
        errExit("sigaction");
    }

    sigemptyset(&emptyMask);

    for (;;) {
        sigsuspend(&emptyMask);

        if (mq_notify(mqd, &sev) == -1) {
            errExit("mq_notify");
        }

        while ((numRead = mq_receive(mqd, buffer, attr.mq_msgsize, 
            NULL)) > 0) {
            printf("Read %ld bytes\n", (long)numRead);

            if (errno != EAGAIN) {
                errExit("mq_receive");
            }
        }
    }
}