#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include <assert.h>

#include "utils.h"

int main(int argc, const char **argv) {
    /* Broker */
    const char *hostname;
    int port;

    /* TCP Connection with AMQP Broker */ 
    amqp_socket_t *socket;
    amqp_connection_state_t conn;

    /* Time to Happen */
    struct timeval tval;
    struct timeval *tv;

    /* At least 3 command line arguments */
    if (argc < 3) {
        /* Not enough arguments */
        fprintf(stderr, "Usage: amqp_connect_timeout host port [timeout_sec "
            " [timeout_usec=0]]\n");

        return 1;
    }

    if (argc > 3) {
        /* Timeout sec value ... String to Integer */
        tv = &tval;

        tv->tv_sec = atoi(argv[3]);

        if (argc > 4) {
            /* Timeout usec value */
            tv->tv_usec = atoi(argv[4]);
        } else {
            tv->tv_usec = 0;
        }
    } else {
        tv = NULL;
    }

    hostname = argv[1];
    port = atoi(argv[2]);

    conn = amqp_new_connection();
    
    socket = amqp_tc_socket_new(conn);
    if (!socket) {
        die("creating TCP socket");
    }

    die_on_error( amqp_socket_open_noblock(socket, hostname, port, tv), 
        "opeining TCP socket" );

    die_on_amqp_error( amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
        "guest", "guest"), "Logging in" );

    die_on_amqp_error( amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection" );

    die_on_error( amqp_destroy_connection( conn ), "Ending connection");

    printf("Done\n");

    return 0;
}