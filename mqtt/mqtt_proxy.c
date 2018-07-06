/*
 * gcc -I/usr/local/include -L/usr/local/lib/ -o select_mqtt_async_PROXY select_mqtt_async_PROXY.c -lpaho-mqtt3a
 */

#include "MQTTAsync.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define DEVICE_PATH "/sys/class/leds"

/* Control variables */
volatile int finished = 0;
char *topic = NULL;
int subscribed = 0;
int disconnected = 0;
volatile int toStop = 0;

/* Pipe to communicate with select() and local event loop */
/* Global Variable - ALL functions can access the pipe */
int pfd[2];

/* 
 * MQTT Connection for SUBscribe
 */
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
    "stdout-subscriber-async", 1, '\n', 2, NULL, NULL, "localhost", "1883", 0, 10
};

/*
 * MQTT Connection for PUBlish
 */
struct {
    char *clientid;
    char *delimiter;
    int maxdatalen;
    int qos;
    int retained;
    char *username;
    char *password;
    char *host;
    char *port;
    int verbose;
    int keepalive;
} pub_opts = {
    "stdin-publisher-async", "\n", 100, 0, 0, NULL, NULL, "localhost", "1883", 0, 10 
};

MQTTAsync client;
    
void cfinish(int sig) {
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

/* 
 * This is a callback function ... The client application must provide
 * an implementation of this function to enable Asynchronous receipt
 * of messages.
 */
int 
messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    int rc = 0;

    if (opts.showtopics) {
        printf("%s\t", topicName);
    }
    
    if (opts.nodelimiter) {
        printf("%.*s", 
            message->payloadlen, (char *)message->payload);
    } else {
        printf("%.*s%c", 
            message->payloadlen, (char *)message->payload, opts.delimeter);
    }

    fflush(stdout);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    //close(pfd[0]);
    write(pfd[1], "Message for Led\n", strlen("Message for Led\n")+1);

    MQTTAsync_responseOptions pub2_opts = MQTTAsync_responseOptions_initializer;
    do {
        rc = MQTTAsync_send(client, topic, sizeof("LED ON\n"), "LED ON\n", pub_opts.qos, pub_opts.retained, &pub2_opts);
    } while (rc != MQTTASYNC_SUCCESS);

    return 1;
}

MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

void connectionLost(void *context, char *cause) {
    MQTTAsync client = (MQTTAsync) context;
    int rc;

    printf("connectionLost called\n");

    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start reconnect, return code is %d\n", rc);
        finished = 1;
    }
}

void deliveryComplete() {
    ;
}

void onDisconnect(void *context, MQTTAsync_successData *response) {
    disconnected = 1;
}

void onSubscribe(void *context, MQTTAsync_successData *response) {
    subscribed = 1;
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    printf("Subscribe Failed, rc = %d\n", response->code);
    finished = 1;
}

void onConnect(void *context, MQTTAsync_successData *response) {
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
    int rc;

    if (opts.showtopics) {
        printf("Subscribing to topic %s with client %s at QoS %d\n",
            topic, opts.clientid, opts.qos);
    }

    ropts.onSuccess = onSubscribe;
    ropts.onFailure = onSubscribeFailure;
    ropts.context = client;

    if ((rc = MQTTAsync_subscribe(client, topic, opts.qos, &ropts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    printf("Connect Failed, rc = %d\n", response ? response->code : -99);
    finished = 1;
}

/** 
 * Entry Point for PROXY Process - Responsible to:
 * 1) Send local sensor measurement to central monitoring tools,
 * 2) Collect remote control requests to ON/ OFF local actors (leds)   
 */
int 
main(int argc, char *argv[]) 
{
    char url[100];
    int rc = 0;

    fd_set readfds, writefds, exceptfds;

    //int pfd[2];

    int nfds;
    struct timeval *pto;
    int ready;

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

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

    if (pipe(pfd) == -1) {
        /* FAILURE : Create PIPE to local event loop */ 
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* Timeout value for select()/ monitor multiple */
    /* NULL means no timeout */
    pto = NULL;
    
    /* Largest file descriptor in all sets ... INITIAL value */
    nfds = 0;

    /* File Descriptor Sets for ... Input, Output and Exceptional Events */
    FD_ZERO( &readfds );
    FD_ZERO( &writefds );
    FD_ZERO( &exceptfds );

    /* Monitor read-end of pipe ... wake up when new data available */ 
    FD_SET(pfd[0], &readfds);
    nfds = MAX( nfds , pfd[0] + 1);

    /* Monitor write-end of pipe ... wake up when all data is sent and buffer empty again */
    FD_SET(pfd[1], &writefds);
    nfds = MAX( nfds, pfd[1] + 1);

    /* Create an MQTT client ready for connection to specified broker */
    rc = MQTTAsync_create(&client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTASYNC_SUCCESS) {
        perror("mqttasync_create");
        exit(EXIT_FAILURE);
    }

    /* 
     * Set the global callback functions for specific client:
     * 1) onConnectionLost
     * 2) onMessageArrived
     * 3) onDeliveryComplete
     * 
     * Needs a pointer to the application context
     */
    MQTTAsync_setCallbacks(
        client, client, 
        connectionLost, messageArrived, 
        deliveryComplete );

    signal(SIGINT, cfinish);
    signal(SIGTERM, cfinish);

    /* 
     * Define several settings that control the way the client 
     * connects to an MQTT server 
     */
    
    /* the time period without data exchange between client and server */
    conn_opts.keepAliveInterval = opts.keepalive;

    /* boolean value : behavior of client & server at conenction and disconnection 
     * When is true, the state information is discarded 
     * When is false, the next time the server checks for previous session information 
     */
    conn_opts.cleansession = 1;
    conn_opts.username = opts.username;
    conn_opts.password = opts.password;

    /* A pointer to a callback function ... if connect() successfuly completed */
    conn_opts.onSuccess = onConnect;
    /* A callback function ... if connect() failure */    
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;

    /* Attempts to connect a previously created client to an 
     * MQTT server/ borker ising the specified options */
    if (
        (rc = MQTTAsync_connect(client, &conn_opts)) 
            != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    /* Never use this kind of code with PIPES 
     * **************************************
    if ((fd = open(pfd[1], O_WRONLY)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    } 
    * ********************************************
    * Already configured for exclusive read/ write
    */
    
    /* Check Subscription Status to Topic */
    while (!subscribed) {
        /*
         * Not yet successful subscription ... Try again later 
         */
        usleep(10000L);
    }
    
    /* Application Terminated ... Ctrl + C */
    if (finished) {
        goto exit;
    }

    int ii = 0;
    char readbuffer[80];
    int nbytes;

    while (!finished) {
        ready = select(nfds, &readfds, NULL, NULL, pto);

        if (ready == -1) {
            /* An error occured */
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (ready == 0) {
            /* Call was timed out */
            ;
        } 
        
        printf("**Wake Up***%d\n", ++ii);

        //close(pfd[1]);
        nbytes = read(pfd[0], readbuffer, sizeof(readbuffer));

        printf("%d: %s", nbytes, readbuffer);

        
        usleep(10000L);
    }

    disc_opts.onSuccess = onDisconnect;
    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    while( !disconnected ) {
        usleep(10000L);
    }

exit:
        MQTTAsync_destroy(&client);

        return EXIT_SUCCESS;
}