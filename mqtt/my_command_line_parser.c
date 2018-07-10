#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "my_command_line_parser.h"

/**
 * Button
 **/
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
                pbutton_uv->device = (char*) malloc( sizeof(char) * strlen(optarg) );
                strcpy( pbutton_uv->device, optarg );

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

/**
 * LED options
 **/
int 
argsparse_led(int argc, char **argv, struct led_user_values *pled_uv) {
    int c;

    while (1) 
    {
        /* getopt_long stores the option idex here */
        int option_index = 0;

        c = getopt_long(argc, argv, optstring_led, long_options_led, &option_index);

        /* Detect the end of the options */
        if (c == -1) {
            break;
        }

        switch (c) 
        {
            case 0:
                /* If this option set a flag, do nothing else mow */
                if (long_options_led[option_index].flag != 0) {
                    break;
                }

                printf("option %s", long_options_led[option_index].name);

                if (optarg) {
                    printf(" with arg %s", optarg);
                }

                printf("\n");
                break;

            case 'q':
                pled_uv->qos = atoi(optarg);
                break;
            
            case 'h':
                pled_uv->host = (char *)malloc( sizeof(char) * strlen(optarg) );
                strcpy( pled_uv->host, optarg);
                break;

                pled_uv->topic = (char*) malloc( sizeof(char) * strlen(optarg) );
                strcpy( pbutton_uv->topic, optarg );

                puts("option -a\n");
                break;

    int count = 2;

    while (count < argc) {
        if (strcmp(argv[count], "--qos") == 0) {

            if (++count < argc) {
                
                if (strcmp(argv[count], "0") == 0) {
                    opts.qos = 0;
                }
        
                else if (strcmp(argv[count], "1") == 0) {
                    opts.qos = 1;
                }
        
                else if (strcmp(argv[count], "2") == 0) {
                    opts.qos = 2;
                }
            }
            else {
                usage();
            }
        }
        else if (strcmp(argv[count], "--host") == 0) {
            if (++count < argc) {
                opts.host = argv[count];
            } else {
                usage();
            }
        }
        else if (strcmp(argv[count], "--port") == 0) {
            if (++count < argc) {
                opts.port = argv[count];
            } else {
                usage();
            }
        }   
        else if (strcmp(argv[count], "--username") == 0) {
            if (++count < argc) {
                opts.username = argv[count];
            } else {
                usage();
            }
        }
        else if (strcmp(argv[count], "--password") == 0) {
            if (++count < argc) {
                opts.password = argv[count];
            } else {
                usage();
            }
        }
        else if (strcmp(argv[count], "--delimiter") == 0) {
            if (++count < argc) {
                if (strcmp("newline", argv[count]) == 0) {
                    opts.delimeter = '\n';
                } else {
                    opts.delimeter = argv[count][0];
                }
                opts.nodelimiter = 0;
            } else {
                usage();
            }
        }
        else if (strcmp(argv[count], "--showtopics") == 0) {
            if (++count < argc) {
                if (strcmp(argv[count], "on") == 0) {
                    opts.showtopics = 1;
                } else if (strcmp(argv[count], "off") == 0) {
                    opts.showtopics = 0;
                }
            } else {
                usage();
            }
        }
        else if (strcmp(argv[count], "--keepalive") == 0) {
            if (++count < argc) {
                opts.keepalive = atoi(argv[count]);
            } else {
                usage();
            }
        }

        count++;
    }
} 
