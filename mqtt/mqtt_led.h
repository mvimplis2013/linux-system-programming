#ifndef MQTT_LED_H
#define MQTT_LED_H

#include <stdlib.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define LOG_LEVEL LOG_DEBUG

#define DEVICE_PATH "/sys/class/leds/input2::numlock/brightness"

const int NO_DELIMETER = 1;
const int QOS = 2 ;
const int SHOW_TOPICS = 0;
const int KEEP_ALIVE = 10;

const struct 
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
    NO_DELIMETER, '\n', QOS, 
    NULL, NULL, 
    "localhost", "1883", 
    SHOW_TOPICS, KEEP_ALIVE
};

#endif