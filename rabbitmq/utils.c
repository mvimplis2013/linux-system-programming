#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <amqp.h>

#include "utils.h"

void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

void die_on_amqp_error(amqp_rpc_reply_t x, char const *context) {
    switch (x.reply_type) {
        case AMQP_RESPONSE_NORMAL:
            return;
        case AMQP_RESPONSE_NONE:
            fprintf(stderr, "%s: missing RPV reply type!\n", context);
            break;
        case AMQP_RESPONSE_LIBRARY_EXCEPTION:
            fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x.library_error));
            break;
        case AMQP_RESPONSE_SERVER_EXCEPTION:
            switch (x.reply.id) {
                case AMQP_CONNECTION_CLOSE_METHOD:
                    amqp_connection_close_t *m = (amqp_connection_close_t *)x.reply_decoded;
                    fprintf(stderr, "%s: server connection error %uh, message: %.*s\n",
                        context, m->reply_code, (int)m->reply_text.len, 
                        (char *)m->reply_text.bytes );
                    break;
            }
    }
}