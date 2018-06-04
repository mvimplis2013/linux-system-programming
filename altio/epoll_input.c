/*
 * epoll_input.c
 * 
 * Example of the use of the Linux epoll API.
 * 
 * Usage: epoll_input file...
 * 
 * This program opens all of the files named in its command-line arguments
 * and monitors the resulting file descriptors for input events.
 * 
 * This program is Linux (2.6 and later) specific.
 * */
#include <sys/epoll.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define MAX_BUF 1000                    // Maximum bytes fetched by a single read()
#define MAX_EVENTS 5                    // Maximum number of events to be returned from a single epoll_wait() call

