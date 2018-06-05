/*
 * pmsg_create.c
 * 
 * Create a POSIX message queue.
 * 
 * Usage: as shown in usageError()
 * 
 * Linux supports POSIX message queues since kernel 2.6.6.
 */
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tlpi_hdr.h"
