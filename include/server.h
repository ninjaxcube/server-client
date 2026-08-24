#ifndef SERVER_H
#define SERVER_H
#include <stdint.h>
#include <poll.h>

#define MAX_CLIENTS 20
#define MAX_MESSAGE_LENGTH 1000
#define PORT "4040"
#define IP_ADDRESS "127.0.0.1"
#define TIMEOUT 1000

typedef int8_t socket_t;
typedef uint8_t byte_t;
typedef struct __attribute__((packed)) message
{
    uint32_t id;
    uint32_t length;
    char data[];
} message_t;

typedef struct poll_set
{
    struct pollfd * fds;
    nfds_t nfds;
} poll_set_t;

socket_t tcp_listener(const char * ip_address, const char * port, uint8_t max_clients);
int8_t server_send(socket_t client_socket, message_t * p_msg);
int8_t init_poll_set(poll_set_t * p_poll_set, nfds_t max_fds);
int8_t add_poll_fd(poll_set_t * p_poll_set, socket_t socket);
int8_t remove_poll_fd(poll_set_t * p_poll_set, socket_t socket);
int8_t destroy_poll_set(poll_set_t * p_poll_set);
int8_t server_receive(socket_t client_socket, byte_t * p_buffer, message_t * p_message);

#endif