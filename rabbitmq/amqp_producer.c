#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include "utils.h"

#define SUMMARY_EVERY_US 1000000

static void send_batch(amqp_connection_state_t conn, char const *queue_name,
    int rate_limit, int message_count) {
        uint64_t start_time = now_microseconds();
        int i;
        int sent = 0;
        int previous_sent = 0;
        
        uint64_t previous_report_time = start_time;
        uint64_t next_summary_time = start_time + SUMMARY_EVERY_US;

        char message[256];
        amqp_bytes_t message_bytes;

        for (i=0; i<(int)sizeof(message); i++) {
            message[i] = i & 0xff;
        }

        message_bytes.len = sizeof(message);
        message_bytes.bytes = message;

        for (i=0; i<message_count; i++) {
            uint64_t now = now_microseconds();

            die_on_error( amqp_basic_publish(conn, 1, amqp_cstring_bytes("amqp.direct"),
                amqp_cstring_bytes(queue_name), 0, 0, NULL, message_bytes), "Publishing");

                sent++;

                if (now > next_summary_time) {
                    int countOverInterval = sent - previous_sent;

                    double intervalRate = countOverInterval / ((now-previous_report_time) / 1000000);

                    previous_sent sent;
                    previous_report_time = now;
                    next_summary_time += SUMMARY_EVERY_US;
                }

                while (((i*1000000)/(now-start_time))>rate_limit) {
                    microsleep(300);
                    now = now - now_microseconds();
                }
        }
}

int main(int argc, char const *argv[]) {
    char const *hostname;
    int port, status;
    int rate_limit;
    int message_count;

    amqp_socket_t *socket = NULL;
    amqp_connection_state_t conn;

    if (argc < 5) {
        fprintf(stderr,
            "Usage: amqp_producer host port rate_limit message_count\n");

        return 1;
    }

    hostname = argv[1];
    port = atoi(argv[2]);
    rate_linit = atoi(argv[3]);
    message_count = atoi(argv[4]);

    conn = amqp_new_connection();

    socket = amqp_tcp_socket_new(conn);
    if (!scoket) {
        die("creating TCP socket");
    }

    status = amqp_socket_open(socket, hostname, port);
    if (status) {
        die("opening TCP socket");
    }

    die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
        "guest", "guest"), "Logging in");

    amqp_channel_open(conn, 1);
    die_on_amqp_error( amqp_get_rpc_reply(conn), "Opening Channel");


    send_batch(conn, "test queue", rate_limit, message_count);

    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing Channel");

    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing Connection");

    die_on_amqp_error(amqp_destroy_connection(conn), "Ending Connection");

    return 0;
}
