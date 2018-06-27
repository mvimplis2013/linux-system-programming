#ifndef librabbitmq_examples_utils_h
#define librabbitmq_examples_utils_h

void die(const char *fmt, ...);

extern void die_on_amqp_error(amqp_rpc_reply_t x, char const *context);

#endif