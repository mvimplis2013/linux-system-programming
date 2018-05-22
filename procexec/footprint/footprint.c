#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

static int
func(int arg)
{
  int j;

  for (j=0; j<0x100; j++)
    if (malloc(0x8000) == NULL) {
	fprintf(stderr, "%s\n", "malloc");
	_exit(EXIT_FAILURE);
      }

  printf("Program break in child: %10p\n", sbrk(0));
      
  return arg;
}
  
int
main(int argc, char *argv[])
{
  //int arg = (argc > 1) ? getInt(argv[1], 0, "arg") : 0;
  pid_t childPid;
  int status;

  setbuf(stdout, NULL);

  printf("Program break in parent: %10p\n", sbrk(0));

  childPid = fork();
  if (childPid == -1) {
    fprintf(stderr, "%s\n", "fork");
    _exit(EXIT_FAILURE);
  }

  // Child calls func() and uses return value as exit statsu
  if (childPid == 0)
    exit(func(100)); //exit(func(arg));

  // Parent waits for child to terminate. It can determine the result of
  // func() by inspecting `status`
  if (wait(&status) == -1){
    fprintf(stderr, "%s\n", "wait");
    _exit(EXIT_FAILURE);
  }

  printf("Program break in parent: %10p\n", sbrk(0));

  printf("Status = %d %d\n", status, WEXITSTATUS(status));

  exit(EXIT_SUCCESS);
}
