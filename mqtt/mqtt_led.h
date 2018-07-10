#ifndef MQTT_LED_H
#define MQTT_LED_H

#include <stdlib.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define LOG_LEVEL LOG_DEBUG

#define DEVICE_PATH "/sys/class/leds/input2::numlock/brightness"

int NO_DELIMETER = 1;
int QOS = 2 ;
int SHOW_TOPICS = 0;
int KEEP_ALIVE = 10;

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
    "led-subscriber-async-01A", 
    1, '\n', 2, 
    NULL, NULL, 
    "localhost", "1883", 
    0, 10
};

#endif