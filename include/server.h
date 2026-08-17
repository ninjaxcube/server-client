#ifndef SERVER_H
#define SERVER_H
#include <stdint.h>

#define MAX_CLIENTS 100
#define MAX_MESSAGE_LENGTH 1000
#define PORT 4040
#define IP_ADDRESS "127.0.0.1"

typedef int8_t socket_t;
typedef uint8_t byte_t;
typedef struct __attribute__((packed)) message
{
    uint32_t id;
    uint32_t length;
    char data[];
} message_t;

socket_t listener();
int8_t server_send(socket_t client_socket, message_t * p_msg);
int8_t server_receive(socket_t client_socket, message_t * p_msg);

#endif