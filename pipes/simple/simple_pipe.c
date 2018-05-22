/* simple_pipe.c
   Simple demonstration of the use of a pipe to communicate between a parent
   and a child process.

   Usage: simple_pipe "string"

   The program creates a pipe, and then calls fork() to create a child process.
   After the fork(), the parent writes the string given on the command line to
   the pipe, and the child uses a loop to read data from the pipe and print it
   on standard output.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 10

int
main(int argc, char *argv[])
{
  int pfd[2];
  char buf[BUF_SIZE];
  ssize_t numRead;

  if (argc != 2|| strcmp(argv[1], "--help") == 0) {
    fprintf(stderr, "%s string\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (pipe(pfd) == -1) {
    fprintf(stderr, "%s\n", "pipe");
    exit(EXIT_FAILURE);
  }

  switch (fork()) {
  case -1:
    fprintf(stderr, "%s\n", "fork");
    exit(EXIT_FAILURE);    
  case 0:
    /* Child - Reads from pipe */
    if (close(pfd[1]) == -1) {
      /* Write end is unused - FAILURE */
      fprintf(stderr, "%s\n", "close-child");
      _exit(EXIT_FAILURE);
    }

    for (;;) {
      /* Read data from pipe , echo on stdout */
      numRead = read(pfd[0], buf, sizeof(buf));

      if (numRead == -1) {
	fprintf(stderr, "%s\n", "read");
	_exit(EXIT_FAILURE);
      }

      if (numRead == 0)
	/* End of file */
	break;

      if (write(STDOUT_FILENO, buf, numRead) != numRead)
	//fatal("child-partial/ failed write");
	//terminate(True);
	;
    }

    write(STDOUT_FILENO, "\n", 1);

    if (close(pfd[0]) == -1) {
      fprintf(stderr, "%s\n", "close");
      _exit(EXIT_FAILURE);
    }

    _exit(EXIT_SUCCESS);

  default:
    /* Parent writes to pipe */
    if (close(pfd[0]) == -1) {
      /* Read end is unused */
      fprintf(stderr, "%s\n", "close-parent");
      exit(EXIT_FAILURE);
    }

    if(write(pfd[1], argv[1], strlen(argv[1])) != strlen(argv[1])) {
      //fatal("parent - partial/failed write");
      // terminate(True);
      ;
    }

    /* Child will see EOF */
    if (close(pfd[1]) == -1) {
      fprintf(stderr, "%s\n", "close");
      exit(EXIT_FAILURE);
    }
      
    /* Wait for child to finish */
    wait(NULL);
	
    exit(EXIT_SUCCESS);
  }
}
    
  
   
