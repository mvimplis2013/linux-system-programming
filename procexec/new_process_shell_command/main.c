#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define SHELL "bin/sh"
#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE);} while(0);

int
my_system(const char *command)
{
  int status;
  pid_t pid;

  int link[2];
  char foo[4096];

  if (pipe(link) == -1)
    die("pipe");
  
  pid = fork();
  if (pid == 0) {
    /* This is the child process. Execute the shell command. */

    dup2(link[1], STDOUT_FILENO);
    close(link[0]);
    close(link[1]);
    
    
    //execl(SHELL, SHELL, "-c", command, NULL);
    execl("/bin/ls", "ls", "-1", (char *)0);
    _exit(EXIT_FAILURE);
  }
  else if (pid < 0)
    /* The fork failed. Report failure. */
    status = -1;
  else {
    /* This is the parent process. Wait the child to complete. */

    close(link[1]);
    int nbytes = read(link[0], foo, sizeof(foo));
    printf("Output: %.*s\n", nbytes, foo);

    wait(NULL);
    if (waitpid(pid, &status, 0) != pid)
      status = -1;
  }

  return status;
}

int
main(int argc, const char *argv[]) {
  const char *command = argv[1];

  my_system(command);
}
