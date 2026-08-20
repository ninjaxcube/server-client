#include "../include/server.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>

int main()
{
    byte_t buffer[sizeof(message_t) + MAX_MESSAGE_LENGTH] = {0};
    message_t p_msg = {0};
    socket_t listener_socket = listener();

    if (-1 == listener_socket)
    {
        fprintf(stderr, "Failed to create listener socket\n");
        return -1;
    }

    socket_t client_fd = accept(listener_socket, NULL, NULL);
    
    while(1)
    {
        server_receive(client_fd, buffer, &p_msg);
        printf("Received message with ID: %u, Length: %u, Data: %s\n"
        ,p_msg.id, p_msg.length, p_msg.data);
    }
    return 0;
}