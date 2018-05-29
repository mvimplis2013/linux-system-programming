/*  test_become_daemon.c
    Test our becomeDaemon() function.
*/
#include "become_daemon.h"
#include "tipi_hdr.h"

int
main(int argc, char *argv[]) {
  becomeDaemon(0);

  /* Normally a daemon would live for ever; we just sleep for a while */
  sleep((argc > 1) ? getInt(argv[1], GN_GT_0, "sleep-time"): 20);

  exit(EXIT_SUCCESS);
}

  
