/*
 * sig_receiver.c
 * 
 * Usage: sig_receiver [block-time]
 * 
 * Cactch and report statistics on signals sent by sig_sender.c.
 * 
 * Note that although we use signal() to establish the signal
 * handler in this program, the use of sigaction() is always 
 * strongly preferable for this task.
 */
#include <signal.h>

#include "signal_functions.h"
#include "tlpi_hdr.h"

static int sigCnt[NSIG];                        /* Counts deliveries of each signal */
static volatile sig_atomic_t getSigint = 0;     /* Set nonzero if SIGINT is delivered */ 

static void 
handler(int sig) 
{
    if (sig == SIGINT) {
        gotSigint = 1;
    } else {
        sigCnt[sig]++;
    }
}

int main(int argc, char *argv[]) {
    int n, numSecs;
    sigset_t pendingMask, blockingMask, emptyMask;

    printf("%s: PID is %ld\n", argv[0], (long)getpid());

    /* Here we use the simpler signal() API to establish a 
     * signal handler, but sigaction() API is strongly 
     * preferred for this task.
     */
    for (n=1; n<NSIG; n++) {
        /* Same handler for all signals */
        (void) signal(n, handler);  /* Ignore Errors */
    } 

    /* If a sleep time was specified, temporary block all 
     * signals, sleep (while another process sends us signals),
     * and then display the mask of pending signals 
     * and unblock all signals.
     */
    if (argc > 1) {
        numSecs = getInt(argv[1], GN_GT_0, NULL);

        sigfillset(&blockingMask);
        if (sigprocmask(SIG_SETMASK, &blockingMask, NULL) == -1) {
            errExit("sigprocmask");
        }

        printf("%s: sleeping for %d seconds\n", argv[0], numSecs);
        sleep(numSecs);

        if (sigpending(&pendingMask) == -1) {
            errExit("sigpending");
        }

        printf("%s: pending signals are: \n", argv[0]);
        printSigset(stdout, "\t\t", &pendingMask);

        sigempyset(&emptySet);      /* Unblock all signals */
        if (sigprocmask(SIG_SETMASK, &emptyMask, NULL) == -1) {
            errExit("sigprocmask");
        }

        while (!gotSigint) {
            continue;
        }

        for (n=1; n<NSIG; n++) {
            if (sigCnt[n] != 0) {
                printf("%s: signal %d caught %d times\n",
                    argv[0], n, sigCnt[n]);
            }
        }

    }

    exit(EXIT_SUCCESS);
}