/*  become_daemon.c
    A function encapsulating the steps in becoming a daemon.
*/
int                          /* Returns 0 on success, -1 on error */
becomeDaemon(int flags)
{
  int maxfd, fd;

  switch (fork()) {          /* Become background process */
  case -1: return -1;
  case 0:  break;            /* Child process through ... */
  default: _exit(EXIT_SUCCESS);
  }

  if (setsid() == -1)        /* Become leader of new session */
    return -1;
  
    
