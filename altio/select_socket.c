#include <unistd.h>
#include <fcntl.h>

#include <sys/select.h>

int 
main(int argc, char *argv[]) {

    /* Set nonblock for stdin */
    int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flag);

    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    printf("Waiting for server message or stdin input. Please, type text to send:\n");

    // server socket always will be greater the STDIN_FILENO
    int maxfd = server.socket();

    while (1) {
        // SELECT updates fd_set's, so we need to build fd_set's 
        // before each SELECT call
        build_fd_sets(&server, &read_fds, &write_fds, &except_fds);
    }
}