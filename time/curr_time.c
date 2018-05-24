/* curr_time.c
   
   Implement our currTime() function.
*/

#include <time.h>
#include "curr_time.h"

#define BUF_SIZE 1000

/* Return a string containing the current time formatted according to 
   the specification in `format`. 
   If `format` is NULL, we use [%c] as a default specifier - which gives the 
   date and time as for ctime(3), but without the trailing newline).
   Return NULL on error. */

char *
currTime(const char *format)
{
  static char buf[BUF_SIZE];
  time_t t;
  size_t s;
  struct tm *tm;

  t = time(NULL);
  tm = localtime(&t);
  if (tm == NULL)
    return NULL;

  s = strftime(buf, BUF_SIZE, (format != NULL) ? format : "%c", tm);

  return (s == 0) ? NULL : buf;
}
