#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include "../include/server.h"


socket_t listener()
{

    struct addrinfo hints, * res, * p;
    int8_t status;
    char ipstr[INET6_ADDRSTRLEN];
    socket_t server_socket = -1;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((0 != (status = getaddrinfo(IP_ADDRESS, "4040", &hints, &res))))
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    for(p = res; NULL != p; p = p->ai_next)
    {
        void * addr;
        if(AF_INET == p->ai_family )
        {
            struct sockaddr_in * ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        }
        else
        {
            struct sockaddr_in6 * ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("Server initialized on: %s\n", ipstr);
    }

    if(-1 == (server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)))
    {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }
    
    if(-1 == fcntl(server_socket, F_SETFL, O_NONBLOCK))
    {
        fprintf(stderr, "fcntl: %s\n", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }

    if(-1 == bind(server_socket, res->ai_addr, res->ai_addrlen))
    {
        fprintf(stderr, "socket: %s\n", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }

    if(-1 == setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)))
    {
        fprintf(stderr, "setsockopt: %s\n", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }

    if(-1 == listen(server_socket, MAX_CLIENTS))
    {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }


    freeaddrinfo(res);
    return server_socket;
}

int8_t server_send(socket_t client_socket, message_t * p_msg)
{
    uint32_t bytes_sent = 0;
    uint32_t bytes_left = p_msg->length + sizeof(message_t);
    int8_t bytes_written = 0;
    
    p_msg->id = htonl(p_msg->id);
    p_msg->length = htonl(p_msg->length);

    while(bytes_sent < bytes_left)
    {
        if(-1 == (bytes_written = send(client_socket,
        (byte_t*)p_msg + bytes_sent, bytes_left - bytes_sent, 0)))
        {
            fprintf(stderr, "send: %s\n", strerror(errno));
            return -1;
        }
        bytes_sent += bytes_written;
    }
    return 0;
}

int8_t server_receive(socket_t client_socket, byte_t * p_buffer, message_t * p_message)
{
    uint32_t bytes_received = 0;
    uint32_t bytes_left = sizeof(message_t);
    int8_t bytes_read = 0;

    while(bytes_received < bytes_left)
    {
        if(-1 == (bytes_read = recv(client_socket,
        p_buffer + bytes_received, bytes_left - bytes_received, 0)))
        {
            fprintf(stderr, "recv: %s\n", strerror(errno));
            return -1;
        }
        else if(0 == bytes_read)
        {
            fprintf(stderr, "Client disconnected\n");
            return -1;
        }
        bytes_received += bytes_read;
    }
    
    memcpy(p_message, p_buffer, sizeof(message_t));
    p_message->id = ntohl(p_message->id);
    p_message->length = ntohl(p_message->length);

    bytes_left = p_message->length;
    bytes_received = 0;

    while(bytes_received < bytes_left)
    {
        if(-1 == (bytes_read = recv(client_socket,
        p_buffer + sizeof(message_t) + bytes_received, bytes_left - bytes_received, 0)))
        {
            fprintf(stderr, "recv: %s\n", strerror(errno));
            return -1;
        }
        else if(0 == bytes_read)
        {
            fprintf(stderr, "Client disconnected\n");
            return -1;
        }
        bytes_received += bytes_read;
    }

    memcpy(p_message->data, p_buffer + sizeof(message_t), p_message->length);

    return 0;
}