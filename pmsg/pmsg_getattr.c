/* 
 * pmsg_getattr.c
 * 
 * Display attributes of a POSIX message queue.
 * 
 * Linux supports POSIX message queues since kernel 2.6.6.
 * 
 * gcc -I ../lib/ ../lib/error_functions.c ../lib/get_num.c -o pmsg_getattr  pmsg_getattr.c -lrt
 */
#include <mqueue.h>
#include "tlpi_hdr.h"

int 
main(int argc, char *argv[]) 
{
    mqd_t mqd;
    struct mq_attr attr;

    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s mq-name\n", argv[0]);
    } 

    mqd = mq_open(argv[1], O_RDONLY);
    if (mqd == (mqd_t) -1) {
        errExit("mq_open");
    }

    if (mq_getattr(mqd, &attr) == -1) {
        errExit("mq_getattr");
    }

    printf("Manimum # of messages on queue: %ld\n", attr.mq_maxmsg);
    printf("Maximum message size: %ld\n", attr.mq_msgsize);
    printf("# of messages currently on queue: %ld\n", attr.mq_curmsgs);

    exit(EXIT_SUCCESS);
}