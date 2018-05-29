int 
main(int argc, char *argv[]) 
{
    stack_t sigstack;
    struct sigaction sa;
    int j;

    printf("Top of standard stack is near %10p\n", (void *) &j);

    /* Allocate alternate stack and inform kernel of its existence */
    sigstack.ss_sp = malloc(SIFSTKSZ);
    if (sigstack.ss_sp == NULL) 
    errExit("malloc");

    sigstack.ss_size = SIGSTKSZ;

    sigstack.ss_flags = 0;

    if (sigaltstack(&sigstack, NULL) == -1 ) {
        errExit("sigaltstack");
    }

}