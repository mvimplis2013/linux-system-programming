/* demo_sched_fifo.c
   
   This program demonstrates the use of realtime scheduling policies.
   It creates two processes, each running under the SCHED_FIFO scheduling policy.
   Each process executes a function that prints a message every quarter of a second
   of CPU time. After each second of consumed CPU time, the function and calls calls
   sched_yield() to yield the CPU to the other process. Once a process has consumed 
   3 seconds of CPU time, the function terminates.

   This program must be run as a SUPERUSER, or (on Linux 2.6.12 and later) with a suitable
   RLIMIT_RTRIO resource limit,
*/
#define _GNU_SOURCE
#include <sched.h>
#include <sys/resource.h>
#include <sys/times.h>

#include <stdlib.h>
#include <stdio.h>

/* CPU centiseconds between messages */
#define CSEC_STEP 25

int
main(int argc, char *argv[])
{
  struct rlimit rlim;
  struct sched_param sp;
  cpu_set_t set;

  /* Disable buffering of stdout */
  setbuf(stdout, NULL);

  /* Confine all processes to a single CPU, so that the processes won't run in parallel 
     on multi-CPU systems. */
  CPU_ZERO(&set);
  CPU_SET(1, &set);

  if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
    fprintf(stderr, "%s\n", "sched_affinity");
    exit(EXIT_FAILURE);

}
