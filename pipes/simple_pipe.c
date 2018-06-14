/*
 * simple_pipe.c
 * 
 * Simple demonstration of the use of the pipe to communicate between 
 * a parent and a child process.
 * 
 *  Usage: simple_pipe "string"
 * 
 * The program creates a pipe, and then calls fork() to create a child process.
 * After the fork, the parent writes the string given on the command line to the pipe,
 * and the child uses a loop to read data from the pipe and print it on standard output.
 * 
 */
#include <sys/wait.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 10

int 

