#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>

#pragma GCC diagnostic ignored "-Woverride-init"

#include "button_constants.h"

/*char mqtt_msg[1000];*/

void 
handleIOEventCommon(int fd, char *mqtt_msg) 
{
    int rd;
	struct input_event ev[64];
    /*int i;*/
    size_t i;

	char msg_1[300];
	char msg_2[300];
	char msg_3[300];

    printf ( "Hello Miltos\n" );

    rd = read( fd, ev, sizeof(struct input_event)*64 );
		
    for (i=0; i<rd/sizeof(struct input_event); i++) 
    {
        if (ev[i].type == EV_SYN) 
        {
            sprintf(msg_1, 
                "Event: time %ld.%06ld, -------- %s --------\n",
                ev[i].time.tv_sec, ev[i].time.tv_usec,
                ev[i].code ? "Config Sync" : "Report Sync");
        } 
        else if (ev[i].type == EV_MSC && 
                (ev[i].code == MSC_RAW || ev[i].code == MSC_SCAN)) 
        {
            sprintf(msg_2,
				"Event: time %ld.%06ld, type %d (%s), code %d (%s), value %02x\n",
                ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
                events[ev[i].type] ? events[ev[i].type] : "?",
                ev[i].code,
                names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?",
                ev[i].value);
        } 
        else {
            sprintf(msg_3,
				"Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
                ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
                events[ev[i].type] ? events[ev[i].type] : "?",
                ev[i].code,
                names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?",
                ev[i].value);                    
        }
    }

    sprintf(mqtt_msg, "%s", msg_1);
    sprintf(mqtt_msg + strlen(mqtt_msg), "%s", msg_2);
    sprintf(mqtt_msg + strlen(mqtt_msg), "%s", msg_3);
}


int
handleKeyboardIOEvent(int fd_key, char *mqtt_msg) 
{
    handleIOEventCommon(fd_key, mqtt_msg);
    
    printf("%s\n", mqtt_msg);

    return 1;
}

int 
handleMouseIOEvent(int fd_mouse, char *mqtt_msg) 
{
    handleIOEventCommon(fd_mouse, mqtt_msg);

    printf("%s\n", mqtt_msg);

    return 1;
}