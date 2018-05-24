/* fork_sig_sync.c

   Demonstrates how signals can be used to synchronize the actions of a 
   parent and child process.
*/
#include <signal.h>

/* Declaration of currTime() */
#include "curr_time.h"

/* Synchronization signal */
#define SYNC_SIG SIGUSR1

static void
handler(int sig)
{
  /* Signal handler - does nothing but return */
}

int main(int argc, char *argv[])
{
  pid_t childPid;
  sigset_t blockMask, origMask, emptyMask;
  struct sigaction sa;

  /* Disable buffering of stdout */
  setbuf(stdout, NULL);

  sigemptyset(&blockMask);
  /* Block signal */
  sigaddset(&blockMask, SYNC_SIG);

  if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1) {
    fprintf(stderr, "%s\n", "sigprocmask");
    exit(EXIT_FAILURE);
  }

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handles = handler;
  if (sigaction(SYNC_SIG, &sa, NULL) == -1) {
    fprintf(stderr, "%s\n", "sigaction");
    exit(EXIT_FAILURE);
  }

  switch (childPid = fork()) {
  case -1:
    fprintf(stderr, "%s\n", "fork");
    exit(EXIT_FAILURE);
  case 0:
    /* Child */

    /* Child does some required action here ...*/
    printf("[%s %ld] Child started - doing some work\n",
	   currTime("%T"), (long) getpid());

    /* Simulate time sepnt doing some work */
    sleep(2);

    /* And then signals parent that it's done */
    printf("[%s %ld] Parent got signal\n", currTime("#T"), (long) getpid());

    if (kill(getppid(), SYNC_SIG) == -1)
      {
	fprintf(stderr, "%s\n", "kill");
	exit(EXIT_FAILURE);
      }

    /* Now child can do other things ... */
    _exit(EXIT_SUCCESS);
  default:
    /* Parent */

    /* Parent may do some work here, and then waits for child to complete
       the required action. */
    printf("[%s %ld]Parent about to wait for signal\n",
	   currTime("%T"), (long) getpid());
    sigemptyset(&emptyMask);
    if (sigsuspend(&emptyMask) == -1 && errno != EINTR) {
      fprintf(stderr, "%s\n", "sigsuspend");
      exit(EXIT_FAILURE);
    }

    /* If required, return signal mask to its original state */
    if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1) {
      fprintf(stderr, "%s\n", "sigprocmask");
      exit(EXIT_FAILURE);
    }

    /* Parent carries on to do other things ... */
    exit(EXIT_SUCCESS);
  }
}
