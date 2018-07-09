#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "my_command_line_parser.h"

int
argsparse_button(int argc, char **argv, struct button_user_values *pbutton_uv)
{
    int c;

    while (1) 
    {
        /* getopt_long stores the option idex here */
        int option_index = 0;

        c = getopt_long(argc, argv, optstring_button, long_options_button, &option_index);

        /* Detect the end of the options */
        if (c == -1) {
            break;
        }

        switch (c) 
        {
            case 0:
                /* If this option set a flag, do nothing else mow */
                if (long_options_button[option_index].flag != 0) {
                    break;
                }

                printf("option %s", long_options_button[option_index].name);

                if (optarg) {
                    printf(" with arg %s", optarg);
                }

                printf("\n");
                break;

            case 't':
                pbutton_uv->topic = (char*) malloc( sizeof(char) * strlen(optarg) );
                strcpy( pbutton_uv->topic, optarg );

                puts("option -a\n");
                break;

            case 'd':
                puts("option -b\n");
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