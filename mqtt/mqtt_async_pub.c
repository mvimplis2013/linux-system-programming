/*
 * stdin publisher
 * 
 * compulsory parameters:
 * 
 * --topic topic to publish on
 * 
 * defaulted parameters:
 * 
 *  --host :: localhost
 *  --port :: 1883
 *  --qos :: 0
 *  --delimiters :: \n
 *  --clientid :: stdin-publisher-async
 *  --maxdatalen :: 100
 *  --keepalive :: 10
 * 
 *  --userid :: none
 *  --password :: none
 *  
 * gcc -I/usr/local/include -L/usr/local/lib/ mqtt_aync_sub.c -lpaho-mqtt3a
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <MQTTAsync.h>

struct {
    char *clientid;
    char *delimeiter;
    int maxdatalen;
    int qos;
    int retained;
    char *username;
    char *password;
    char *host;
    char *port;
    int verbose;
    int keepalive;
} opts = {
    "stdin-publisher-async", "\n", 100, 0, 0, NULL, NULL, "localhost", "1883", 0, 10 
};

void getopts(int argc, char **argv);
void myconnect(MQTTAsync *client);

void usage(void) {
    printf("MQTT stdin publisher\n");
    printf("Usage: stdinpub topicname <options>, where options are:\n");
    printf("    --host <hostname> (default is %s)\n", opts.host);
    printf("    --port <port> (default is %s)\n", opts.port);
    printf("    --qos <qos> (default is %d)\n", opts.qos);
    printf("    --retained (default is %s)\n", opts.retained ? "on" : "off");
    printf("    --delimiter (default is \\n)\n");
    printf("    --clientid <clientid> (defaults is %s)\n", opts.clientid);
    printf("    --maxdatalen <bytes> (default is %d)\n", opts.maxdatalen);
    printf("    --username none\n");
    printf("    --password none\n");
    printf("    --keepalive <seconds> (default is 10 seconds)\n");

    exit(EXIT_FAILURE);
}

volatile int toStop = 0;

void cfinish(int sig) {
    signal(SIGINT, NULL);
    toStop = 1;
}

static int connected = 0;

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    printf("Connect failed, rc is %d\n", response ? response->code : -1);
    connected = -1;

    MQTTAsync client = (MQTTAsync)context;
    myconnect(client);
}

void connectionLost(void *context, char *cause) {
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    int rc = 0;

    printf("Connecting\n");
}

void onConnect(void *context, MQTTAsync_successData *response) {
    printf("Connected");
    connected = 1;
}

void myconnect(MQTTAsync *client) {
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    int rc = 0;

    printf("Connecting\n");
    conn_opts.keepAliveInterval = opts.keepalive;
    conn_opts.cleansession = 1;
    conn_opts.username = opts.username;
    conn_opts.password = opts.password;

    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;

    conn_opts.context = client;

    ssl_opts.enableServerCertAuth = 0;
    conn_opts.automaticReconnect = 1;

    connected = 0;

    if ((rc = MQTTAsync_connect(*client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);        
    }
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *m) {
    /* Not expecting any messages */
    return 1;
}

/*
 * Main() 
 */
int main(int argc, char **argv) {
    char url[100];
    char *topic = NULL;
    int rc = 0;
    MQTTAsync client;

    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;

    if (argc < 2) {
        usage();
    }

    getopts(argc, argv);

    sprintf(url, "%s:%s", opts.host, opts.port);

    if (opts.verbose) {
        printf("Broker URL is %s\n", url);
    }

    topic = argv[1];
    printf("Using Topic %s\n", topic);

    create_opts.sendWhileDisconnected = 1;
    rc = MQTTAsync_createWithOptions(&client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);

    signal(SIGINT, cfinish);
    signal(SIGTERM, cfinish);

    rc = MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, NULL);

    myconnect(&client);
}
/*
 * End of Main()
 */

void getopts(int argc, char **argv) {
    int count = 2;

    while (count < argc) {
        if (strcmp(argv[count], "--retained") == 0) {
            opts.retained = 1;
        }

        if (strcmp(argv[count], "--verbose") == 0) {
            opts.verbose = 1;
        }

        else if (strcmp(argv[count], "--qos") == 0) {
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
                else {
                    usage();
                }
            } else {
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
        else if (strcmp(argv[count], "--clientid") == 0) {
            if (++count < argc) {
                opts.clientid = argv[count];
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
        else if (strcmp(argv[count], "--maxdatalen") == 0) {
            if (++count < argc) {
                opts.maxdatalen = atoi(argv[count]);
            } else {
                usage();
            }
        }
        else if (strcmp(argv[count], "--delimiter") == 0) {
            if (++count < argc) {
                opts.delimeiter = argv[count];
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