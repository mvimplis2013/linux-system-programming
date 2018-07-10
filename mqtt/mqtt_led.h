#ifndef MQTT_LED_H
#define MQTT_LED_H

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define LOG_LEVEL LOG_DEBUG

#define DEVICE_PATH "/sys/class/leds/input2::numlock/brightness"

const char *CLIENT_ID = ;
const int nodelimiter;
const char *delimeter;
const int qos;
const char *username;
const char *password;
const char *host;
const char *port;
const int showtopics;
const int keepalive;

#endif