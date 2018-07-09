#ifndef MY_COMMAND_LINE_PARSER_H
#define MY_COMMAND_LINE_PARSER_H

 static struct option long_options[] = {
            /* These options set a flag */
            {"verbose", no_argument, &verbose_flag, 1},
            {"brief",   no_argument, &verbose_flag, 0},
            /* These options do not set a flag. We distinguish them by their indices */
            {"add", no_argument, 0, 'a'},
            {"append", no_argument, 0, 'b'},
            {"delete", required_argument, 0, 'd'},
            {"create", required_argument, 0, 'c'},
            {"file",   required_argument, 0, 'f'},
            {0, 0, 0, 0}
        };
int
argsparse_button(int argc, char **argv);

#endif