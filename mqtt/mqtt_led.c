/*
 * gcc -I/usr/local/include -L/usr/local/lib/ -o select_mqtt_async_sub_led select_mqtt_async_sub_led.c -lpaho-mqtt3a
 */

/* Needed for Nanosleep - Hope Robert will NOT kill me */
#define _POSIX_C_SOURCE 199309L

#include "MQTTAsync.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h> 

#include <syslog.h>

#include <sys/select.h>

#include <time.h>

#include "mqtt_led.h"

/* Pipe File Descriptots : 0/Read-end & 1/Write-end */
int pfd[2];

struct 
{
    char *clientid;
    int nodelimiter;
    char delimeter;
    int qos;
    char *username;
    char *password;
    char *host;
    char *port;
    int showtopics;
    int keepalive;
} opts = {
    "led-subscriber-async-01A", 1, '\n', 2, NULL, NULL, "localhost", "1883", 0, 10
};

volatile int finished = 0;
char *topic = NULL;
int subscribed = 0;
int disconnected = 0;

volatile int toStop = 0;

void cfinish(int sig) {
    syslog(LOG_DEBUG, "New Signal Raised %d\n", sig);

    signal(SIGINT, NULL);
    finished = 1;
}

void usage(void) {
    printf("MQTT stdout subscriber\n");
    printf("Usage: stdoutsub topicname <options>, where options are:\n");
    printf("--host <hostname> (default is \"%s\")\n", opts.host);
    printf("--port <port> (default is \"%s\")\n", opts.port);
    printf("--qos <qos> (default is \"%d\")\n", opts.qos);
    printf("--delimiter <delim> (default is NO delimietr)\n");
    printf("--clientid <client-id> (default is \"%s\")\n", opts.clientid);
    printf("--username none\n");
    printf("--password none\n");
    printf("--showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
    printf("--keepalive <seconds> (default is \"%d\")\n", opts.keepalive);

    exit(EXIT_FAILURE);
}

void getopts(int argc, char **argv) {
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

MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

void connectionLost(void *context, char *cause) {
    MQTTAsync client = (MQTTAsync) context;
    int rc;

    /*printf("connectionLost called\n");*/
    syslog(LOG_DEBUG, "connectionLost called .. %s\n", cause);

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start reconnect, return code is %d\n", rc);
        finished = 1;
    }
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    syslog(LOG_DEBUG, "OnMessageArrived: context = %p , len = %d\n", context, topicLen);

    if (opts.showtopics) {
        /* printf("%s\t", topicName); */
        syslog(LOG_DEBUG, "%s\t", topicName);
    }
    
    if (opts.nodelimiter) {
        /*printf("%.*s", message->payloadlen, (char *)message->payload);*/
        syslog(LOG_DEBUG, "%.*s", message->payloadlen, (char *)message->payload);
    } else {
        /*printf("%.*s%c", message->payloadlen, (char *)message->payload, opts.delimeter);*/
        syslog(LOG_DEBUG, "%.*s%c", message->payloadlen, (char *)message->payload, opts.delimeter);
    }

    fflush(stdout);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    write(pfd[1], "Message for LED\n", sizeof("Message for Led\n"));

    return 1;
}

void onDisconnect(void *context, MQTTAsync_successData *response) {
    syslog(LOG_DEBUG, "OnDisconnect: context = %p , qos = %d\n", context, response->alt.qos);

    disconnected = 1;
}

void onSubscribe(void *context, MQTTAsync_successData *response) {
    syslog(LOG_DEBUG, "OnSubscribe: context = %p , qos = %d\n", context, response->alt.qos);
    subscribed = 1;
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    syslog(LOG_DEBUG, "Subscribe Failed, context = %p , rc = %d\n", context, response->code);

    finished = 1;
}

void onConnect(void *context, MQTTAsync_successData *response) {
    MQTTAsync client_conn = (MQTTAsync)context;
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
    int rc;

    if (opts.showtopics) {
        syslog(LOG_DEBUG, "Subscribing to context = %p , topic %s with client %s at QoS %d , MQTTVer = %d\n", context, 
            topic, opts.clientid, opts.qos, response->alt.connect.MQTTVersion);
    }

    ropts.onSuccess = onSubscribe;
    ropts.onFailure = onSubscribeFailure;
    ropts.context = client_conn;

    if ((rc = MQTTAsync_subscribe(client_conn, topic, opts.qos, &ropts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    syslog(LOG_DEBUG, "OnConnectFailure ... context = %p , rc = %d\n", context, response ? response->code : -99);

    finished = 1;
}

void handleLedOnRequest() {
    /* Open LED Device File */
    int fd; 

    if ((fd = open(DEVICE_PATH, O_RDWR)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    } 
}

/** ************** LED Process *****************
 ** ******************************************** **/
int main(int argc, char *argv[]) 
{
    MQTTAsync client;
    char url[100];
    int rc = 0;

    fd_set readfds;

    int nfds;
    struct timeval *pto;
    int ready;

    /* Sleep for half a second */
    const struct timespec requested_time[] = {{0, 50000000L}};
    struct timespec *remaining = NULL;

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

    /* ************************* SYSLOG Setup **************************  */
	/* *****************************************************************  */
	/* Open a connection to system logger for Button-Handler */ 
	setlogmask( LOG_UPTO( LOG_LEVEL) );
	/* Write to system console if error sending to system logger ... LOG_CONS
	 * Open connection immediately not when first message logged ... LOG_NDELAY
	 * Include caller's PID with each message ... LOG_PID
	 * 
	 * openlog may or may not open the /dev/log socket, depending on option. If it does, it tries to open it and connect it as a stream socket.
	 */  
	openlog ( "controller-handler", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	/*const char *format = "%syslogpriority%,%syslogfacility%,%timegenerated%,%HOSTNAME%,%syslogtag%,%msg%\n"; */
	syslog (LOG_DEBUG, "Button Handler is Launching");

    if ( argc<2 ) {
        usage();
    }

    topic = argv[1];

    if (strchr(topic, '#') || strchr(topic, '+')) {
        opts.showtopics = 1;
    } 
    if (opts.showtopics) {
    }

    getopts(argc, argv);
    sprintf(url, "%s:%s", opts.host, opts.port);

    /* Pipe .. connect MQTT-Lights-on messages to LED event loop */
    if (pipe(pfd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* No select() timeout */ 
    pto = NULL;
    
    /* Initial value for select monitored max file-descriptor */
    nfds = 0;

    /* Initialize file-descriptor sets for INPUT events */
    FD_ZERO(&readfds);
    
    /* Add the pipe read-end in INPUT event fds */
    FD_SET(pfd[0], &readfds);
    nfds = MAX( nfds , pfd[0] + 1);

    rc = MQTTAsync_create(&client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, NULL);

    signal(SIGINT, cfinish);
    signal(SIGTERM, cfinish);

    conn_opts.keepAliveInterval = opts.keepalive;
    conn_opts.cleansession = 1;
    conn_opts.username = opts.username;
    conn_opts.password = opts.password;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        /*printf("Failed to start connect, return code %d\n", rc);*/

        syslog(LOG_DEBUG, "Failed to start connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    while (!toStop) {
        /* 
         * I/O Multiplexing .. 
         * Simultaneous monitor many file descriptors and see if 
         * read/ write is possible on any of them. 
         */

        /* ************ End of select() system call **********
         * ******* to monitor multiple file descriptors ****** 
         */

        ready = select(nfds, &readfds, NULL, NULL, pto);

        if (ready == -1) {
            /* An error occured */
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (ready == 0) {
            /* Call was timed out */
            perror("timeout");
            exit(EXIT_FAILURE);
        } 
        
        else if (ready == 1) {
            /* the monitored pipe has new mqtt-light-on request */
            handleLedOnRequest();
        }

        /* Ready for Select() Wakeup ... Device File is changed */        
    }

    while (!subscribed) {
        /*usleep(10000L);*/ 

        if (finished) {
            goto exit;
        }

        while (!finished) {
            /* usleep(10000L); */
        }

        disc_opts.onSuccess = onDisconnect;
        if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) {
            syslog(LOG_DEBUG, "Failed to start disconnect, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }

        while( !disconnected ) {
            /*usleep(10000L);*/
            nanosleep( requested_time, remaining);
        }
    }

exit:
        MQTTAsync_destroy(&client);

        return EXIT_SUCCESS;
}