#include "../include/server.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
    
volatile int8_t g_running = 1;

inline static void close_all_sockets(poll_set_t * p_poll_set)
{
    if(NULL == p_poll_set || NULL == p_poll_set->fds)
    {
        return;
    }

    for(nfds_t i = 0; i < p_poll_set->nfds; i++)
    {
        close(p_poll_set->fds[i].fd);
    }
}

void stop_server(int signum)
{
    (void)signum;
    g_running = 0;
}

int main()
{
    byte_t buffer[sizeof(message_t) + MAX_MESSAGE_LENGTH + 1] = {0};
    message_t * p_msg = (message_t*)buffer;
    socket_t listener_socket = tcp_listener(IP_ADDRESS, PORT, MAX_CLIENTS);
    poll_set_t poll_set = {0};


    if(-1 == listener_socket)
    {
        fprintf(stderr, "tcp_listener: %s\n", strerror(errno));
        return -1;
    }
    
    if(-1 == init_poll_set(&poll_set, MAX_CLIENTS))
    {
        fprintf(stderr, "init_poll_set: %s\n", strerror(errno));
        return -1;
    }

    if(-1 == fcntl(listener_socket, F_SETFL, O_NONBLOCK))
    {
        fprintf(stderr, "fcntl: %s\n", strerror(errno));
        goto cleanup;
    }

    if (-1 == listener_socket)
    {
        fprintf(stderr, "Failed to create listener socket\n");
        goto cleanup;
    }

    if(-1 == add_poll_fd(&poll_set, listener_socket))
    {
        fprintf(stderr, "failed to add listener socket to poll set..\n");
        goto cleanup;
    }

    printf("Server started.\nUse CTRL+C to exit.\n");

    struct sigaction sa;
    sa.sa_handler = stop_server;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 
    sigaction(SIGINT, &sa, NULL);
    
    while(1)
    {
        if (0 == g_running)
        {
            goto cleanup;
        }

        int8_t socket_event = poll(poll_set.fds, poll_set.nfds, TIMEOUT);
        if(0 != socket_event && EINTR != errno)
        {
            fprintf(stderr, "poll: %s. %d\n", strerror(errno), errno);
            goto cleanup;
        }

        else if (socket_event)
        {
            for (nfds_t i = 0; i < poll_set.nfds; i++)
            {
                if (poll_set.fds[i].revents & POLLIN)
                {
                    if(poll_set.fds[i].fd == poll_set.fds[0].fd)
                    {
                        socket_t client_socket = accept(poll_set.fds[0].fd, NULL, NULL);
                        if(-1 == client_socket && (EAGAIN != errno || EWOULDBLOCK != errno))
                        {
                            fprintf(stderr, "accept: %s\n", strerror(errno));
                            goto cleanup;
                        }

                        if(-1 == fcntl(client_socket, F_SETFL, O_NONBLOCK))
                        {
                            fprintf(stderr, "fcntl: %s\n", strerror(errno));
                            goto cleanup;
                        }

                        printf("New client connected: %d\n", client_socket);

                        if(-1 == add_poll_fd(&poll_set, client_socket))
                        {
                            fprintf(stderr, "add_poll_fd: %s\n", strerror(errno));
                            goto cleanup;
                        }
                    }
                    else
                    {
                        socket_t client_socket = poll_set.fds[i].fd;
                        if(-1 == server_receive(client_socket, buffer))
                        {
                            fprintf(stderr, "server_receive: %s\n", strerror(errno));
                            remove_poll_fd(&poll_set, client_socket);
                            close(client_socket);
                            continue;
                        }

                        printf("Received message from client %d: ID=%u, Length=%u, Data=%s\n",
                            client_socket, p_msg->id, p_msg->length, p_msg->data);

                        if(-1 == server_send(client_socket, p_msg))
                        {
                            fprintf(stderr, "server_send: %s\n", strerror(errno));
                            remove_poll_fd(&poll_set, client_socket);
                            close(client_socket);
                            continue;
                        }
                    }
                }
            }
        }
    }

    cleanup:
        printf("\nShutting down server...\n");
        close_all_sockets(&poll_set);
        destroy_poll_set(&poll_set);
        return 0;
}