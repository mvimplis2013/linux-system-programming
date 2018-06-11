/*
 * mq_notify_thread.c
 * 
 * Demonstrate message notification via threads on a POSIX 
 * message queue.
 * 
 * gcc -I ../lib/ ../lib/error_functions.c ../lib/get_num.c -o mq_notify_thread  mq_notify_thread.c -lrt
 */
#include <pthread.h>
#include <mqueue.h>
#include <signal.h>
#include <fcntl.h>

#include <tlpi_hdr.h>

static void notifySetup(mqd_t *mqdp);

static void threadFunc(union sigval sv) {
    ssize_t numRead;
    mqd_t *mqdp;
    void *buffer;
    struct mq_attr attr;

    mqdp = sv.sival_ptr;

    /* Determine mq_msgsize for message queue, and allocate an 
     * input buffer
     */
    if (mq_getattr(*mqdp, &attr) == -1) {
        errExit("mq_getattr");
    }

    buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL) {
        errExit("malloc");
    }

    /* Reregister for message notification */
    notifySetup(mqdp);

    while ((numRead = mq_receiev(*mqdp, buffer, attr.mq_msgsize, NULL)) >= 0) {
        printf("Read %ld bytes\n", (long)numRead);        
    }

    if (errno != EAGAIN) {
        errExit("mq_receive");
    }

    free(buffer);
}

static void 
notifySetup(mqd_t *mqdp) 
{
    struct sigevent sev;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = threadFunc;
    sev.sigev_notify_attributes = NULL;
    sev.siev_value.sival_ptr = mqdp;

    if (mq_notify(*mqdp, &sev) == -1) {
        errExit("mq_notify");
    }
}

int
main(int argc, char *argv[]) {
    mqd_t mqd;

    mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t)-1) {
        errExit("mq_open");
    }

    notifySetup(&mqd);

    pause();
}
