/*
 * gcc -I/usr/local/include -L/usr/local/lib/ mqtt_aync_sub.c -lpaho-mqtt3a
 */

#include "MQTTAsync.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/select.h>

/* Logmask for Informational, Debug and Error Messages */
#include <syslog.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define DEVICE_PATH "/sys/class/leds"

#define LOG_LEVEL LOG_DEBUG
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
    "stdout-subscriber-async", 1, '\n', 2, NULL, NULL, "localhost", "1883", 0, 10
};

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

volatile int finished = 0;
char *topic = NULL;
int subscribed = 0;
int disconnected = 0;

volatile int toStop = 0;

MQTTAsync client;
    
void cfinish(int sig) {
    syslog( LOG_DEBUG, "Signal Received %d\n", sig);

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
    MQTTAsync client_lost = (MQTTAsync) context;

    int rc;

    /* printf("connectionLost called\n"); */
    syslog( LOG_DEBUG, "Connection Lost , cause = %s\n", cause);

    if ((rc = MQTTAsync_connect(client_lost, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start reconnect, return code is %d\n", rc);
        finished = 1;
    }
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    MQTTAsync client_msg = (MQTTAsync) context;
    int rc = 0;
    MQTTAsync_responseOptions pub2_opts = MQTTAsync_responseOptions_initializer;
    
    if (opts.showtopics) {
        /* printf("%s\t", topicName); */
        syslog(LOG_DEBUG, "Topic: %s[%d]\t", topicName, topicLen);
    }
    
    if (opts.nodelimiter) {
        /*printf("%.*s", message->payloadlen, (char *)message->payload);*/
        syslog( LOG_DEBUG, "%.*s", message->payloadlen, (char *)message->payload);
    } else {
        /*printf("%.*s%c", message->payloadlen, (char *)message->payload, opts.delimeter);*/
        syslog( LOG_DEBUG, "%.*s%c", message->payloadlen, (char *)message->payload, opts.delimeter);
    }

    fflush(stdout);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    write(pfd[1], "Message for LED\n", sizeof("Message for Led\n"));

    do {
        rc = MQTTAsync_send(client_msg, topic, sizeof("LED ON\n"), "LED ON\n", pub_opts.qos, pub_opts.retained, &pub2_opts);
    } while (rc != MQTTASYNC_SUCCESS);

    return 1;
}

void onDisconnect(void *context, MQTTAsync_successData *response) {
    syslog(LOG_DEBUG, "OnDisconnect: context: %p , qos: %d\n", context, response->alt.qos);
    disconnected = 1;
}

void onSubscribe(void *context, MQTTAsync_successData *response) {
    syslog(LOG_DEBUG, "OnSubscribe: context: %p , qos: %d\n", context, response->alt.qos);
    subscribed = 1;
}

void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    /*printf("Subscribe Failed, rc = %d\n", response->code);*/
    syslog(LOG_DEBUG, "OnSubscribe: context: %p , rc: %d\n", context, response->code);

    finished = 1;
}

void onConnect(void *context, MQTTAsync_successData *response) {
    MQTTAsync client_conn = (MQTTAsync)context;
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
    int rc;

    if (opts.showtopics) {
        /*printf("Subscribing to topic %s with client %s at QoS %d\n", topic, opts.clientid, opts.qos);*/
        syslog( LOG_DEBUG, "Subscribing to topic %s with client %s at QoS %d , %d\n", topic, opts.clientid, opts.qos, response->alt.qos);
    }

    ropts.onSuccess = onSubscribe;
    ropts.onFailure = onSubscribeFailure;
    ropts.context = client;

    if ((rc = MQTTAsync_subscribe(client_conn, topic, opts.qos, &ropts)) != MQTTASYNC_SUCCESS) {
        /*printf("Failed to start subscribe, return code %d\n", rc);*/
        syslog( LOG_DEBUG, "Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    /*printf("Connect Failed, rc = %d\n", response ? response->code : -99);*/
    syslog( LOG_DEBUG, "Connect Failed, context = %p , rc = %d\n", context, response ? response->code : -99);

    finished = 1;
}

int main(int argc, char *argv[]) 
{
    char url[100];
    int rc = 0;

    fd_set readfds, writefds;

    /*int pfd[2];*/

    int nfds;
    struct timeval *pto;
    int fd;
    int ready;

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

    if (pipe(pfd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pto = NULL;
    
    nfds = 0;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    FD_SET(pfd[0], &readfds);
    nfds = MAX( nfds , pfd[0] + 1);

    FD_SET(pfd[1], &writefds);
    nfds = MAX( nfds, pfd[1] + 1);

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
        printf("Failed to start connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    /* Open Device File */
    if ((fd = open(DEVICE_PATH, O_RDONLY)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    } 
    
    while (!toStop) {
        /* int data_len = 0;*/

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
            ;
        } 
        
        printf("**************************\n");
        
        /*else if (ready == 1) {
            // The device file descriptor block released
        }*/

        /* Ready for Select() Wakeup ... Device File is changed */        
    }

    while (!subscribed) {
        usleep(10000L);

        if (finished) {
            goto exit;
        }

        while (!finished) {
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
    }

exit:
        MQTTAsync_destroy(&client);

        return EXIT_SUCCESS;
}