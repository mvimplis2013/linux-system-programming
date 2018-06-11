/*
 * gcc -I/usr/local/include -L/usr/local/lib/ mqtt_aync_sub.c -lpaho-mqtt3a
 */

#include "MQTTAsync.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

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

volatile int finished = 0;
char *topic = NULL;
int subscribed = 0;
int disconnected = 0;

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

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    if (opts.showtopics) {
        printf("%s\t", topicName);
    }
    
    if (opts.nodelimiter) {
        printf("%.*s", message->payloadlen, (char *)message->payload);
    } else {
        printf("%.*s%c", message->payloadlen, (char *)message->payload, opts.delimeter);
    }

    fflush(stdout);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return 1;
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
int main(int argc, char *argv[]) 
{
    MQTTAsync client;
    char url[100];
    int rc = 0;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

    if ( argc<2 ) {
        usage();
    }

    topic = argv[1];

    if (strchr(topic, '#') || strchr(topic, '+')) {
        opts.showtopics = 1;
    } 
    if (opts.showtopics) {
        printf("topic is \"%s\"\n", topic);
    }

    getopts(argc, argv);
    sprintf(url, "%s:%s", opts.host, opts.port);

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