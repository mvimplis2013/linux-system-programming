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
argsparse_button(int argc, char **argv);

#endif