#ifndef MY_COMMAND_LINE_PARSER_H
#define MY_COMMAND_LINE_PARSER_H

/* Flag set by '--verbose' */
static int verbose_flag;

static const char *optstring_button = "t:d:";

static const struct option long_options_button[] = {
    /* These options set a flag */
    {"verbose", no_argument, &verbose_flag, 1},
    {"brief",   no_argument, &verbose_flag, 0},
    
    /* These options do not set a flag. We distinguish them by their indices */
    {"topic", required_argument, 0, 't'},
    {"device", required_argument, 0, 'd'},
    {0, 0, 0, 0}
};

struct button_user_values {
    char *topic;
    char *device;
} button_uv;

int 
argsparse_button(int argc, char **argv, struct button_user_values *pbutton_uv);

/*************************************** LED ****************************************/
static const char *optstring_led = ":q:h:p:u:pwd:d:s:k:";

static const struct option long_options_led[] = {
    /* These options set a flag */
    {"verbose", no_argument, &verbose_flag, 1},
    {"brief",   no_argument, &verbose_flag, 0},
    
    /* These options do not set a flag. We distinguish them by their indices */
    {"qos", optional_argument, 0, 'q'},
    {"host", optional_argument, 0, 'h'},
    {"port", optional_argument, 0, 'p'},
    {"user", optional_argument, 0, 'u'},
    {"pass", optional_argument, 0, 'pwd'},
    {"delimeter", optional_argument, 0, 'd'},
    {"show-topics", optional_argument, 0, 's'},
    {"keep-alive", optional_argument, 0, 'k'},
    {0, 0, 0, 0}
};

struct led_user_values {
    int verbose = 1;
    int qos;
    char *host;
    int port;
    char *user;
    char *pass;
    char delimeter;
    int show_topics;
    int keep_alivel;
    char *topic;
    char *device;
} led_uv;

int 
argsparse_led(int argc, char **argv, struct led_user_values *pled_uv);

#endif