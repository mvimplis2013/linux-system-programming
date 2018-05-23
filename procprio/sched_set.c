/* sched_set.c

   Usage: sched_set policy priority pid...
   
   Sets the policy and priority of all processes specified by the 'pid' arguments.

   See also sched_view.c
*/
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <errno.h>

#include <limits.h>

#define GN_ANY_BASE 0100
#define GN_BASE_8 0200
#define GN_BASE_16 0400

#define GN_NONNEG 01
#define GN_GT_0 02

static void
gnFail(const char *fname, const char *msg, const char *arg, const char *name) {
  fprintf(stderr, "%s error", fname);

  if (name != NULL)
    fprintf(stderr, " (in %s)", name);

  fprintf(stderr, ": %s\n", msg);

  if (arg != NULL && *arg != '\0')
    fprintf(stderr, "   offending text: %s\n", arg);

  exit(EXIT_FAILURE);
}

/* Convert a numeric command-line argument ('arg') into a long integer,
   returned as the function result. 'flags' is a bit mask of flags controlling 
   how the conversion is done and what diagnostic checks are performed on the 
   numeric result.
   'fname' is the name of our caller, and 'name' is the name associated with the command-line
   argument 'arg'.
 */
static long
getNum(const char *fname, const char *arg, int flags, const char *name)
{
  long res;
  char *endptr;
  int base;

  if (arg == NULL || *arg == '\0')
    gnFail(fname, "null or empty string", arg, name);

  base = (flags & GN_ANY_BASE) ? 0 : (flags & GN_BASE_8) ? 8 : (flags & GN_BASE_16) ? 16 : 10;

  errno = 0;
  res = strtol(arg, &endptr, base);

  if (errno != 0)
    gnFail(fname, "strtol() failed", arg, name);

  if (*endptr != '\0')
    gnFail(fname, "nonnumeric characters", arg, name);

  if ((flags & GN_NONNEG) && res < 0)
    gnFail(fname, "negative value not allowed", arg, name);

  if ((flags & GN_GT_0) && res <= 0)
    gnFail(fname, "value must be > 0", arg, name);

  return res;
}

int
getInt(const char *arg, int flags, const char *name)
{
  long res;

  res = getNum("getInt", arg, flags, name);

  if (res > INT_MAX || res < INT_MIN)
    gnFail("getInt", "integer out of range", arg, name);
  
  return (int)res;
}

int main(int argc, char *argv[])
{
  int j, pol;
  struct sched_param sp;

  if (argc < 3 || strchr("rfobi", argv[1][0]) == NULL) {
    fprintf(stderr, "%s policy priority [pid...]\n"
	    "  policy is 'r' (RR), 'f' (FIFO), 'b' (BATCH), 'i' (IDLE), or 'o' (OTHER)\n",
	    argv[0]);

    fflush(stderr);
    exit(EXIT_FAILURE);
  }

  pol = (argv[1][0] == 'r') ? SCHED_RR :
    (argv[1][0] == 'f') ? SCHED_FIFO :
    (argv[1][0] == 'b') ? SCHED_BATCH :
    (argv[1][0] == 'i') ? SCHED_IDLE : SCHED_OTHER;

  sp.sched_priority = getInt(argv[2], 0, "priority");
  
}
