/* 
 * The following example changes the signal mask
 */
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

void catcher(int sig) 
{
    printf("inside catcher() function\n");
}

int main(int argc, char *argv[])
{
    time_t start, finish;
    struct sigaction sact;
    sigset_t new_set, old_set;
    double diff;

    sigemptyset( &sact.sa_mask );

    sact.sa_flags = 0;
    sact.sa_handler = catcher;
    sigaction(SIGALRM, &sact, NULL);

    sigemptyset( &new_set );
    sigaddset( &new_set, SIGALRM );
    sigprocmask(SIG_BLOCK, &new_set, &old_set);

    time( &start );
    printf("SIGALRM signals blocked at %s\n", ctime(&start));

    alarm(1);               /* SIGAARM will be sent in 1 second */

    do {
        time( &finish );
        diff = difftime( finish, start );
    } while (diff < 10);

    sigprocmask( SIG_SETMASK, &old_set, NULL );
    printf("SIGALRM signals unblocked at %s\n", ctime(&finish));

    return 0;
}