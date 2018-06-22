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
 * gcc -I/usr/local/include -L/usr/local/lib/ -o mqtt_button mqtt_button.c -lpaho-mqtt3a
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/select.h>

/* struct input_event */
#include <linux/input.h>

/* Logmask for Informational, Debug and Error Messages */
#include <syslog.h>

#include <MQTTAsync.h>

#include "handle_sensor_io_events.h"

#define DEVICE_PATH "/dev/input/event2"
#define DEVICE_PATH_MOUSE "/dev/input/event3"

#define LOG_LEVEL LOG_DEBUG

#define MAX(X, Y) (X > Y ? X : Y)

/* Global Status Variables for Different Application Stages */
MQTTAsync client;
/* for do {} while -- How many attempts to succeed */
static int counter = 0;

static int is_delivered = 0;
static int connected = 0;	/* that's a shared resource */
static int disconnected = 0;	/* that's a shared resource */

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
} opts = {
"stdin-publisher-async", "\n", 100, 0, 0, NULL, NULL, "localhost",
	    "1883", 1, 10
};

void getopts(int argc, char **argv);
void myconnect(MQTTAsync * client);

void usage(void)
{
	printf("MQTT stdin publisher\n");
	printf("Usage: stdinpub topicname <options>, where options are:\n");
	printf("    --host <hostname> (default is %s)\n", opts.host);
	printf("    --port <port> (default is %s)\n", opts.port);
	printf("    --qos <qos> (default is %d)\n", opts.qos);
	printf("    --retained (default is %s)\n",
	       opts.retained ? "on" : "off");
	printf("    --delimiter (default is \\n)\n");
	printf("    --clientid <clientid> (defaults is %s)\n", opts.clientid);
	printf("    --maxdatalen <bytes> (default is %d)\n", opts.maxdatalen);
	printf("    --username none\n");
	printf("    --password none\n");
	printf("    --keepalive <seconds> (default is 10 seconds)\n");

	exit(EXIT_FAILURE);
}

volatile int toStop = 0;	/* why is this volatile and not static ? */

/* this is a signal handler */
void cfinish(int sig)
{
	sig = sig;
	/* you want to send a signal 
	   from a signal handler 
	   which the handler is subscribed to
	   endless loop? */
	signal(SIGINT, NULL);	/* what is this? */

	toStop = 1;		/* this is not necessarily atomic */
}

/* 
 * Callback function. Implement the application logic in case 
 * of Asynchronous Connection-Filure
 */
void onConnectFailure(void *context, MQTTAsync_failureData * response)
{
	//printf("Connect failed, rc is %d\n", response ? response->code : -1);	/* don't use ? - unreadable code */
	connected = -1;		/* not necessarily atomic */
}

/* 
 * This is a callback function. Enable asynchronous notification in 
 * case of successful `connection` completion.
 */
void onConnect(void *context, MQTTAsync_successData * response)
{
	//printf("Connected");

	/* Proceed to next stage ... Have a Connection to Server */ 
	connected = 1;		/* not necessarily atomic */
}

void connectionLost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync) context;
	MQTTAsync_connectOptions conn_opts =
	    MQTTAsync_connectOptions_initializer;
	MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
	int rc = 0;
	ssl_opts = ssl_opts;

	cause = cause;

	printf("Connecting\n");
	conn_opts.keepAliveInterval = 10;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;

	ssl_opts.enableServerCertAuth = 0;
	/*conn_opts.ssl_opts = &ssl_opts; */

	connected = 0;

	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start connect, return code is %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

/* 
 * Establish MQTT Connection of Client with Server/ Broker ...
 * On Top of TCP/ IP stack
 * Client sends a CONNECT message and Server responds with a CONNACK 
 */
void myconnect( MQTTAsync * client )
{
	/* 
	 * struct_id, struct_version, keepAliveInterval, cleansession,
	 * maxInFlight, username, password, connectTimeout, retryInterval,
	 * ssl, onSuccess, onFailure, context
	 */
	MQTTAsync_connectOptions conn_opts =
	    MQTTAsync_connectOptions_initializer;

	/* 
	 * struct_id, struct_version, trustStore, keyStore, privateKey,
	 * privateKeyPassword, enableCipherSuites, enableServerCertAuth
	 */
	MQTTAsync_SSLOptions ssl_opts = 
		MQTTAsync_SSLOptions_initializer;

	int rc;
	
	/* 
	 * Configure Connection Parameters 
	 */
	conn_opts.keepAliveInterval = opts.keepalive;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;

	/* Pointer to callback function if connect SUCCESSFULLY completes */
	conn_opts.onSuccess = onConnect;
	/* Pointer to callback function if connect FAILS */
	conn_opts.onFailure = onConnectFailure;

	/* the context to be passed to callbacks */
	conn_opts.context = client;

	/* No SSL/ TLS connection using the OpenSSL library */ 
	ssl_opts.enableServerCertAuth = 0;
	
	conn_opts.automaticReconnect = 1;
	/*** Finished Connection Options Configuration ***/

	// printf("Connecting\n");
	syslog(LOG_DEBUG, "Ready to Connect to Server\n");

	/*** How Many Attempts to Establish Client-Server Connection ***/
	/* Static/ Global Variable 
	 * Initialize here ... before connect attempts 
	 * Define Later ... onSuccess() callback = 1
	 */  
	connected = 0;

	/* The safe case - Avoid It
	if ( (rc = MQTTAsync_connect(*client, &conn_opts)) 
			!= MQTTASYNC_SUCCESS ) 
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	} */

	counter = 0;
	do {
		/* New Attempt */ 
		counter++;

		if ( (rc = MQTTAsync_connect(*client, &conn_opts)) 
				== MQTTASYNC_SUCCESS ) 
		{
			/* MQTT Server - Client Connection Established */
			break;
		} else {
			/* Log Connection Error Code */
			syslog(LOG_ERR, "MQTT Connection Failure, error code = %d\n", rc);
		} 
	} while ( 1 );
	
	/* Outside execution loop - check how many attempts */
	if (counter > 1) {
		syslog(LOG_ERR, "Many Attempts to Connect: %d\n", counter);
	} else {
		syslog(LOG_DEBUG, "Connected Immediately");
	}
}

int messageArrived(void *context, char *topicName, int topicLen,
		   MQTTAsync_message * m)
{
	context = context;
	topicName = topicName;
	topicLen = topicLen;
	m = m;
	/* Not expecting any messages */
	return 1;
}

/* 
 * This is a callback function. The client application should provide
 * an implementation to enable asynchronous nitification of message 
 * delivery to MQTT Server/ Broker.
 */
void 
deliveryComplete(void *context, MQTTAsync_token token) 
{
	syslog(LOG_DEBUG, "Successful MQTT Message Delivery");
	is_delivered = 1;
}

void onDisconnect(void *context, MQTTAsync_successData * response)
{
	context = context;
	response = response;
	disconnected = 1;
}

static int published = 0;

void onPublish(void *context, MQTTAsync_successData * response)
{
	context = context;
	response = response;
	published = 1;
}

void onPublishFailure(void *context, MQTTAsync_failureData * response)
{
	context = context;
	response = response;
	printf("Published failed, return code is %d\n",
	       response ? response->code : -1);
	published = -1;
}

/* 
 * This function creates an MQTT client ready for connection to 
 * specified broker/ server. 
 * 
 */
void 
tryToCreateClient( char *url ) {
	/* Response Code */
	int rc;

	/* The necessary parameters to create async client ... Initialize & Customize */
	MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
	create_opts.sendWhileDisconnected = 1;
	
	/* Persistence Types :
	 * MQTTCLIENT_PERSISTENCE_DEFAULT ... File system-based 
	 * MQTTCLIENT_PERSITSENCE_NONE ... In case of failure in-flight messages are lost 
	**/
	rc = 0;
	rc = MQTTAsync_createWithOptions(
		&client, url, 
		opts.clientid,
		/* Use in-memory persistence */
		MQTTCLIENT_PERSISTENCE_NONE, 
		NULL,
		&create_opts);
	if (rc != MQTTASYNC_SUCCESS) {
		// MQTT Client Failure
		// perror("mqttasync_createwithoptions()");
		
		syslog(LOG_ERR, "Failed to start CREATE, return code `%d`\n", rc);
		
		/* Cannot creata an MQTT-Client to Broker ... Abord */
		exit( EXIT_FAILURE );
	}

	/* what should be happening with rc? 
	 * Error handling?
	 * since we are no in the loop here, maybe loop and retry? 
	 * (performance) Variable 'rc' is reassigned a value before the old one has been used. 
	 */

	/* I guess here you want to connect SIGINT 
	 * and SIGTERM with the cfinish signal handler
	 */

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	/* After Successful Client Creation ... Assign Glocal Callback Functions */
	rc = 0;
	rc = MQTTAsync_setCallbacks(
		client, 				/* client handle */
		client, 				/* Application-specific context is passed to callbacks */
		connectionLost,			/* Pointer to MQTTAsync_connectionLost() to handle disconnections */
		messageArrived,			/* Pointer to MQTTAsync_messageArrived() to handle reeipt of messages */ 
		deliveryComplete);		/* Pointer to MQTTAsync_deliveryComplete() check for successful delivery */
	if (rc != MQTTASYNC_SUCCESS) {
		// perror("MQTTAsync_setCallbacks()");

		syslog(LOG_ERR, "Failed to Set Callbacks, return code `%d`\n", rc);
		
		/* Cannot Assign Callbacks to Client ... Abord */
		exit( EXIT_FAILURE );
	}
}

void 
tryToDisconnect() {
	int rc = 0;
	
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

	disc_opts.onSuccess = onDisconnect;


	if ((rc =
	     MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) {
		printf("Failed to start disconnect, return code is %d\n", rc);
		exit(EXIT_FAILURE);
	}

	while (!disconnected) {
		/* usleep(10000L); *//* POSIX.1-2008 removes the specification of usleep(). */
		sleep(1);
	}

	MQTTAsync_destroy(&client);
}

void 
tryToSend( const char *topic, int data_len, void *buffer) 
{
	int rc = 0;

	MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
	
	pub_opts.onSuccess = onPublish;
		pub_opts.onFailure = onPublishFailure;

		do {
			rc = MQTTAsync_send(client, topic, data_len, buffer,
					    opts.qos, opts.retained, &pub_opts);
		} while (rc != MQTTASYNC_SUCCESS);
}

/*
 * Main() 
 */
int main(int argc, char **argv)
{
	char url[100];
	char *topic = NULL;
	int rc = 0;
	char *buffer = NULL;

	int delim_len;

	/* The number of bytes read */
	ssize_t rd;

	int i;

	/* select() system call configuration arguments 
	 * ******************************************** */
	struct timeval *pto;

	/* select() ... returns number of ready file descriptors
	 *  0    on timeout
	 * -1   on failure
	 */
	int ready;

	/* File descriptor sets
	 * readfsd      .. set of file descriptors / if INPUT is possible 
	 * writefds     .. set of file descriptors / if OUTPUT is possible
	 * exceptfds    .. set of file descriptors / if EXCEPTIONAL condition occurred
	 */
	fd_set readfds;

	/* nfds .. should be set to the highest-number file descriptor 
	 *         in any of the three-sets plus(+) 1
	 */
	int nfds=0;

	/* An abstract indicator used to access an input/output resource */
	int fd;
	int fd_mouse;

	char mqtt_msg[1000];

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
	openlog ( "button-handler", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	//const char *format = "%syslogpriority%,%syslogfacility%,%timegenerated%,%HOSTNAME%,%syslogtag%,%msg%\n";
	syslog (LOG_DEBUG, "Button Handler is Launching");

	/* First Check ... Necessary Input Parameters Otherwise Handler Cannot Operate */
	if (argc < 2) {
		usage();
	}	

	/* Enough Command Promt Arguments ... Start Parsing Read & Assign to Application Variables */
	/* IMPORTANT MUST CHANGE ... Use GetOpt() instead */
	getopts(argc, argv);

	/* Local Broker URL */
	sprintf(url, "%s:%s", opts.host, opts.port);

	if (opts.verbose) {
		//printf("Broker URL is %s\n", url);
		syslog(LOG_DEBUG, "MQTT Broker URL : `%s`\n", url);
	}

	topic = argv[1];
	printf("Using Topic %s\n", topic);

	/* *********************************************
	 * **** Device Driver File /dev/input/event ****
	 * ***  Start Monitoring with Blocking Read ****
	 * *********************************************/

	/* after connect MQTT is active?
	 * I guess like this the malloc should be before connect 
	 */

	/* Robert : where does opts.maxdatalen come from?
	 * commandline ? 
	 */

	/* Miltos : variable `maxdatalen` defines the size of buffer 
	 * used for storing the new Sensor/ Button data.
	 * The default value is 100
	 * A user-dedined value can be entered with command line --maxdatalen   
	 */
	buffer = malloc( opts.maxdatalen );
	if (buffer == 0) {
		/* Memory Allocation Error -- Cannot Proceed */
		syslog(LOG_ERR, "malloc() for Sensor/ Button Data ... Failure\n");
		exit(EXIT_FAILURE);
	}

    /* ****************************************** */
	/* **** SELECT() & PSELECT() system call **** */
	/* ****************************************** */

	/* Timeout for select() */
	pto = NULL;					/* Infinite timeout */

	/* Initialize FDS */
	/** KEYBOARD **/
	FD_ZERO( &readfds );

	/* Open Device File KEYBOARD */
	if (
		(fd = open(DEVICE_PATH, O_RDONLY)) < 0 ) 
	{
		/** Failure **/
		
		//perror("open");
		syslog(LOG_ERR, "open() kEYBOARD Driver Failure\n");

		/* Abort */
		exit(EXIT_FAILURE);
	} else {
		/** Success **/
		syslog(LOG_DEBUG, "Successful KEYBOARD Device Driver Open\n");
	}

	/* Let SELECT() monitor if iput is possible in Sensor/ Button Driver */
	FD_SET( fd, &readfds);
	/* Highest numbered file descriptor */
	nfds = MAX(nfds, fd);

	/** Open Device File MOUSE **/
	if (
		(fd_mouse = open(DEVICE_PATH_MOUSE, O_RDONLY)) < 0 ) 
	{
		/** Failure **/
		
		//perror("open");
		syslog(LOG_ERR, "open() MOUSE Driver Failure\n");

		/* Abort */
		exit(EXIT_FAILURE);
	} else {
		/** Success **/
		syslog(LOG_DEBUG, "Successful MOUSE Device Driver Open\n");
	}

	FD_SET( fd_mouse, &readfds);
	nfds = MAX(nfds, fd_mouse);

	printf("nfds = %d\n", nfds);

	nfds += 1;
	syslog(LOG_DEBUG, "Maximum FDS: %d\n", nfds);
	printf("nfds2 = %d\n", nfds);

	/* Is there a specific reason those are not global variables ? */
	/* as global vars we could move MQTT malas in specific functions */

	/* *************************************
	 * ************ MQTT Code **************
	 * *** Create, Configure, Open, Send ***
	 * *************************************/
	 
	/* First Create the MQTT-Client ... Ready to be Connected */
	tryToCreateClient( url );

	/* also this could go into some different function out of main */

	/* what should be happening with rc? 
	 * Error handling?
	 * since we are no in the loop here, maybe loop and retry? 
	 * (performance) Variable 'rc' is reassigned a value before the old one has been used. 
	 */

	/* After successful MQTT-Client Create ... 
	 * **** Try to Esatblish Connection to MQTT-Server/ Broker ****
	 */
	syslog(LOG_DEBUG, "My\n"); 
	myconnect( &client );
	
	printf("HelloMiltos1\n");

	/* Evtest Code Snippet .. */
	while (1) {
		/* Button Process Wait Until New Keystroke */
		//printf("Blocked\n");
        //rd = read( fd, ev, sizeof(struct input_event)*64 );
		//printf("Unblocked\n");

		/** Blocking Read Wake Up **/
	
		/*if ( rd < (int) sizeof(struct input_event) ) {
            //perror("read");
			//printf("Read() Error Value is : %s\n", strerror(errorno));
			
			syslog(LOG_ERR, "Read() Error Value is : %s\n", strerror(errno) ); 
            exit(EXIT_FAILURE);
        } else {
			syslog(LOG_DEBUG, "Read() Sensor Driver : SUCCESS\n");
		}*/

		FD_ZERO( &readfds );
		
		FD_SET( fd, &readfds);
		nfds = MAX(nfds, fd);
	
		FD_SET( fd_mouse, &readfds);
		nfds = MAX(nfds, fd_mouse);

		/*
		FD_ZERO( &exceptfds );
		FD_SET(STDIN_FILENO, &exceptfds);
		FD_SET(server-socket, &exceptfds);

		FD_ZERO( &writefds );
		FD_SET(server-socket, &writefds);
		*/


		/* nfds += 1; */
		ready = select(nfds+1, &readfds, NULL, NULL, pto);
		
        /** New Sensor/ Button Data Arrived **/

		switch (ready) {
			case -1:
				/** An Error Ocred **/

				/* perror("select()"); */
				syslog(LOG_ERR, "SELECT() Call Error Occured!\n");

				/* shutdown_properly(); */
				exit(EXIT_FAILURE);
			case 0:
				/** Call Timeout Before Any FD is Ready **/

				/* printf("SELECT() Call Timeout Reached!\n"); */
				syslog(LOG_ALERT, "SELECT() Call Timeout!\n");

				/* shutdown_properly(); */
				exit(EXIT_FAILURE);

			default: 
				/** One or More File-Descriptors are Ready **/
				syslog(LOG_DEBUG, "Ready File Descriptors #%d\n", ready);

				/** All fd_set's should be checked to find out which 
				 *  I/O events occured 
				 **/
				if ( FD_ISSET(fd, &readfds) ) 
				{
					printf("TTT\n");
					handleKeyboardIOEvent(fd, mqtt_msg);
				}

				if ( FD_ISSET(fd_mouse, &readfds) )
				{
					printf("JJJJ\n");
					handleMouseIOEvent(fd_mouse, mqtt_msg);
				}
		}

		printf("mqtt-msg = %s\n", mqtt_msg);
		tryToSend( topic, strlen(mqtt_msg), mqtt_msg );
    }

	while (!toStop) {
		int data_len = 0;

		/* 
		 * I/O Multiplexing .. 
		 * Simultaneous monitor many file descriptors and see if 
		 * read/ write is possible on any of them. 
		 */

		/* ************ End of select() system call **********
		 * ******* to monitor multiple file descriptors ****** 
		 */

		/* here should be pselect instead of select */
		/* something like:
		   sigaddset(&ss, SIGWHATEVER);
		   ready = pselect(nfds, &readfds, NULL, NULL, pto, &ss);
		 */
		//ready = select(nfds, &readfds, NULL, NULL, pto);

		if (ready == -1) {
			/* An error occured */
			perror("select");
			exit(EXIT_FAILURE);
		}
		/* else */
		if (ready == 0) {
			/* Call was timed out */
			;
		}

		/*else if (ready == 1) {
		   // The device file descriptor block released
		   } */

		/* Ready for Select() Wakeup ... Device File is changed */
		/* char buffer2[] = "Button Pressed\n"; - moved further up */

		/* Read message-to-send from terminal */
		/* int delim_len = 0; *//* why???? */

		/*
		delim_len = (int)strlen(opts.delimiter);

		do {
			buffer[data_len] = buffer2[data_len];
			data_len++;

			if (data_len > delim_len) {
				if (strncmp
				    (opts.delimiter,
				     &buffer[data_len - delim_len],
				     delim_len) == 0) {
					break;
				}
			}
		} while (data_len < opts.maxdatalen);

		if (opts.verbose) {
			printf("Publishing data of length %d\n", data_len);
		}

		tryToSend( topic, data_len, buffer );*/
	}	

	printf("Stopping\n");

	free(buffer);

	tryToDisconnect();

	return EXIT_SUCCESS;
}

/*
 * End of Main()
 */

void getopts(int argc, char **argv)
{
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
				} else if (strcmp(argv[count], "1") == 0) {
					opts.qos = 1;
				} else if (strcmp(argv[count], "2") == 0) {
					opts.qos = 2;
				} else {
					usage();
				}
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--host") == 0) {
			if (++count < argc) {
				opts.host = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--port") == 0) {
			if (++count < argc) {
				opts.port = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--clientid") == 0) {
			if (++count < argc) {
				opts.clientid = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--username") == 0) {
			if (++count < argc) {
				opts.username = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--password") == 0) {
			if (++count < argc) {
				opts.password = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--maxdatalen") == 0) {
			if (++count < argc) {
				opts.maxdatalen = atoi(argv[count]);
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--delimiter") == 0) {
			if (++count < argc) {
				opts.delimiter = argv[count];
			} else {
				usage();
			}
		} else if (strcmp(argv[count], "--keepalive") == 0) {
			if (++count < argc) {
				opts.keepalive = atoi(argv[count]);
			} else {
				usage();
			}
		}

		count++;

	}
}
