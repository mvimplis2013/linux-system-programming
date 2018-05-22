/* vfork_fd_test.c
   Demonstrate that a vfork()-ed child has a separate set of file descriptors
   from its parent.
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>

int
main(int argc, char *argv[]) {
  printf("%s\n", "Hello VFORK()");
  
  switch( vfork() ) {
  case -1:
    printf("%s\n", "vfork");
    _exit(EXIT_FAILURE);
  case 0:
    if (close(STDOUT_FILENO) == -1) {
      printf("%s\n", "close-child");
      _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
  default:
    break;    
  }

  /* Now parent close STDOUT_FILENO twice: only the second close should fail,
     indicating that the close(STDOUT_FILENO) by the child did not affect the 
     parent. */

  printf("%s\n", "Smoother Gybes");
  
  if (close(STDOUT_FILENO) == -1) {
    fprintf(stderr, "%s\n", "close1");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "%s\n", "Windy Gybes");
  perror("Show the Sail");
  
  if (close(STDOUT_FILENO) == -1) {
    fprintf(stderr, "%s\n", "close2");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
  
