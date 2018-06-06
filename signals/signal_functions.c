/*
 * signal_functions.c
 * 
 * Various useful functions for working with signals.
 */
#include <string.h>
#include <signal.h>

#include "signal_functions.h"
#include "tlpi_hdr.h"

void printSigset(FILE *of, const char *prefix, 
    const sigset_t *sigset) 
{
    int sig, cnt;

    cnt = 0;
    for (sig=0; sig<NSIG; sig++) {
        if (sigismember(sigset, sig) {
            cnt++;
            fprintf(of, "%s%d (%s)\n", prefix, sig, strsignal(sig));
        }
    }

    if (cnt == 0) {
        fprintf(of, "%s<emptysignal set>\n", prefix);
    }
}

