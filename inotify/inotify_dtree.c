/*  inotify_dtree.c
    This is an example application to demonstrate the robust use of the inotify API.
    The goal of the application is to maintain an internal representation ("a cache") 
    of the directory tree named on its command line. To keep the application shorter, just 
    the directories are represented, not the files.
    As directories are added, removed, and renamed in the subtrees the resulting inotify events 
    are used to maintain an internal representation of the directory trees that remains consistent
    with the filesystem. The program also provides a command-line interface that allows the user 
    to perform tasks such as dumping the current state of the cache and running a consistency check 
    of the cache against the current state of the directory tree(s).
    The rand_dtree.c program can be used to stress test the operation of this program.
*/

/*  Known limitations
        - Pathnames longer than PATH_MAX are not handled 
*/

#define _GNU_SOURCE 

int 
main(int argc, char *argv[]) {
    fd_set rfds;
    int opt;
    int inotifyFd;
}
    
