#ifndef MESSAGE_H 
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  MAXimum bytes that can be send() or recv() via net by one call.
    It is a good idea to test sending one byte by one.
*/
#define MAX_SEND_SIZE 100

/* Size of Send Queue (how many messages) */
#define MAX_MESSAGES_BUFFER_SIZE 10

#define SENDER_MAXSIZE 128
#define DATA_MAXSIZE 512

#define SERVER_IPV4_ADDR "127.0.0.1"
#define SERVER_LISTEN_PORT 33235

// message ----------------------------------------------------------
typedef struct {
    char sender[SENDER_MAXSIZE];
    char data[DATA_MAXSIZE];
} message_t;

int prepare_message(char *sender, char *data, message_t *message) {
    sprintf(message->sender, "%s", sender);
    sprintf(message->data, "%s", data);

    return 0;
}

int print_message(message_t message) {
    printf("Message: \"%s: %s\"\n", message->sender, message->data);
    return 0;
}

// message queue ----------------------------------------------------
typedef struct {
    int size;
    message_t *data;
    int current;
} message_queue_t;

int create_message_queue(int queue_size, message_queue_t *queue) {
    queue->data = calloc(queue_size, sizeof(message_t));
    queus->size = queue_size;
    queue->current = 0;

    return 0;
}

void delete_message_queue(message_queue_t *queue)
{
    free(queue->data);
    queue->data = NULL;
}