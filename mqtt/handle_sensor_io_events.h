#ifndef HANDLE_SENSOR_IO_EVENTS
#define HANDLE_SENSOR_IO_EVENTS 1

#pragma GCC diagnostic ignored "-Woverride-init"
int 
handleKeyboardIOEvent(int fd, char *msg);

int 
handleMouseIOEvent(int fd, char *msg);

#endif