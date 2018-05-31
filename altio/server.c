int listen_sock;

/* Start listening socket "listen_sock" */
int 
start_listen_socket(int *listen_sock) {
    // Obtain a file descriptor for our "listening" socket.
    *listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*listen_sock < 0) {
        perror("socket");
        return -1;
    }

    int reuse = -1;
    if (setsockopt(*listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
        perror("setsockopt");
        return -1;
    }

    struct sockaddr_in my_addr;
    memset(my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr(SERVER_IPV4_ADDR);
    my_addr.sin_port = htons(SERVER_LISTEN_PORT);

    if (bind(*listen_sock, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) != 0) {
        perror("bind");
        return -1;
    }

    // start accept client connections
    if ( listen(*listen_sock, 10) != 0 ) {
        perror("listen");
        return -1;
    }

    printf("Accepting connections on port %d.\n", (int)SERVER_LISTEN_PORT);

    return 0;
}

int 
build_fd_sets(fd_set &read_fds, fd_set &write_fds)
{
    int i;

    FD_ZERO(read_fds);
    FD_SET(STDIN_FILENO, read_fds);
    FD_SET(listen_socket, read_fds);

    FD_ZERO(write_fds);
}