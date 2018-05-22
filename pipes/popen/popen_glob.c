/* popen_glob.c
   Demonstrates the use of popen() and pclose().

   This program reads filename wildcharacter patterns from standard input and 
   passes each pattern to a popen() call that returns the output from ls(1) for
   the specific wildchard pattern. The program displays the returned output.
*/
int
main(int argc, char *argv[])
{
  /* Pattern for globbing */
  char pat[PAT_SIZE];
  char openCmd[PCDM_BUF_SIZE];
  /* File stream returned by popen */
  FILE *fp;
  /* Invalid characters in `pat` ? */
  Boolean badPattern;
  int len, status, fileCnt, j;
  char pathname[PATH_MAX];

  for (;;) {
    /* Read patterns and display results of globbing. */
    printf("pattern: ");
    fflush(stdout);

    if (fgets(pat, PAT_SIZE, stdin) == NULL)
      /* EOF */
      break;

    len = strlen(pat);
    if (len <= 1)
      /* Empty line */
      continue;

    if (pat[len-1] == '\n')
      /* Strip trailing newlines */
      pat[len-1] = '\0';

    /* Ensure that the prattern contains only valid characters, i.e.,
       letters, digits, underscore, dot, and the shell globbing patterns. */

    for (j=0, badPatterns=FALSE; j<len && !badPatterns; j++) 
      if (!isalnum((unsigned char) pat[j]) &&
	  strchr("_*?[^-].", pat[j]) == NULL)
	badPatterns = TRUE;

    if (badPattern) {
      printf("Bad Pattern character: %c\n", pat[j-1]);
      continue;
    }

    /* Build and execute command to glob `pat` */
    snprintf(popenCmd, PCMD_BUF_SIZE, POPEN_FMT, pat);

    fp = popen(popenCmd, "r");
    if (fp == NULL) {
      printf("popen() failed\n");
      continue;
    }

    /* Read resulting list of pathnames until EOF */
    fileCnt = 0;
    while (fgets(pathname, PATH_MAX, fp) != NULL) {
      printf("%s", pathname);
      fileCnt++;
    }

    /* Close pipe, fetch and display termination status */
    status = pclose(fp);
    printf("%d matching file %s\n", fileCnt, (fileCnt != 1) ? "s" : "");
    printf("pclose() status = %#x\n", (unsigned int) status);
    if (status != -1) {
      printWaitStatus("\t", status);
    }
  }

  exit(EXIT_SUCCESS);
}

      
    
      
  
  
