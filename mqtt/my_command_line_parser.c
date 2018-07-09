#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "my_command_line_parser.h"

/* Flag set by '--verbose' */
static int verbose_flag;

int
argsparse_button(int argc, char **argv)
{
    int c;

    while (1) 
    {
        /* getopt_long stores the option idex here */
        int option_index = 0;

        c = getopt_long(argc, argv, "abc:d:f:", long_options, &option_index);

        /* Detect the end of the options */
        if (c == -1) {
            break;
        }

        switch (c) 
        {
            case 0:
                /* If this option set a flag, do nothing else mow */
                if (long_options[option_index].flag != 0) {
                    break;
                }

                printf("option %s", long_options[option_index].name);

                if (optarg) {
                    printf(" with arg %s", optarg);
                }

                printf("\n");
                break;

            case 'a':
                puts("option -a\n");
                break;
            case 'b':
                puts("option -b\n");
                break;
            case 'c':
                printf("option -c with value `%s`\n", optarg);
                break;
            case 'd':
                printf("option -d with valu `%s`\n", optarg);
                break;
            case 'f':
                printf("option -f with value `%s`\n", optarg);
                break;
            case '?':
                /* getopt_long already printed an error message */
                break;
            default:
                abort();
        }
    }

    /* Instead of reporting --verbose and --brief as they are encountered, we report the final status resulting from them */
    if (verbose_flag) {
        puts("verbose flag is set");
    }

    /* Print any remaining command line arguments (not options) */
    if (optind < argc) {
        printf("non-option ARGV-elements: ");

        while (optind < argc) {
            printf("%s", argv[optind++]);
        }
    }
    
    exit(0);
}