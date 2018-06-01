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

int enqueue(message_queue_t *queue, message_t *message) {
    if (queue->current == queue->size)
        return -1;

    memcpy(&queue->data[queue->current], message, sizeof(message_t));
    queue->current++;

    return 0;
} 

int dequeue(message_queue_t *queue, message_t *message) {
    if (queue->current == 0)
        return -1;
    
    memcpy(message, &queue->data[&queue->current-1], sizeof(message_t));
    queue->current--;

    return 0;
}

int dequeue_all(message_queue_t *queue) {
    queue->current=0;
    return 0;
}

// peer -------------------------------------------------------------------------------------------------------------------------------------
typedef struct {
    int socket;
    struct sockaddr_in addr;

    /* Messages waiting for send */
    message_queue_t send_buffer;

    /*  Buffered sending message
     *
     *  In case we don't send the whole message per one call send()
     *  And current_sending_byte is a pointer to the part of data that will be send next call.
     * */
    message_t sending_buffer;
    size_t current_sending_byte;

    /* The same for the receiving message. */
    message_t receiving_buffer;
    size_t current_receiving_byte;
} peer_t;

int delete_peer(peer_t *peer) {
    close(peer->socket);
    delete_message_queue(&peer->send_buffer);
}

int create_peer(peer_t *peer) {
    create_message_queue(MAX_MESSAGES_BUFFER_SIZE, &peer->send_buffer);

    peer->current_sending_byte = -1;
    peer->current_receiving_byte = 0;

    return 0;
}

char *peer_get_address_str(peer_t *peer) {
    static char ret[INET_ADDRSTRLEN + 10];
    char peer_ipv4_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peer->addr.sin_addr, peer_ipv4_str, INET_ADDRSTRLEN);
    sprintf(ret, "%s:%d\n", peer_ipv4_str, peer->addr.sin_port);

    return ret;
}

int peer_add_to_send(peer_t *peer, message_t *message) {
    return enqueue(&peer->send_buffer, message);
}

/* Receive message from peer and handle it with the message_handler() */
int receive_from_peer(peer_t *peer, int (*message_handler)(message_t))
{
    printf("Ready for recv() from %s.\n", peer_getaddress_str(peer));
}