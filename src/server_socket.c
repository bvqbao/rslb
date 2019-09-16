#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/epoll.h>

#include "epollinterface.h"
#include "connection.h"
#include "logging.h"
#include "netutils.h"
#include "server_socket.h"
#include "queue.h"

#define MAX_LISTEN_BACKLOG 4096


struct proxy_data {
    struct epoll_event_handler* client;
    struct epoll_event_handler* backend;
};


void on_client_read(void* closure, char* buffer, int len)
{
    struct proxy_data* data = (struct proxy_data*) closure;
    if (data->backend == NULL) {
        return;
    }
    connection_write(data->backend, buffer, len);
}


void on_client_close(void* closure)
{
    struct proxy_data* data = (struct proxy_data*) closure;
    if (data->backend == NULL) {
        return;
    }
    connection_close(data->backend);
    data->client = NULL;
    data->backend = NULL;
    epoll_add_to_free_list(closure);
}


void on_backend_read(void* closure, char* buffer, int len)
{
    struct proxy_data* data = (struct proxy_data*) closure;
    if (data->client == NULL) {
        return;
    }
    connection_write(data->client, buffer, len);
}


void on_backend_close(void* closure)
{
    struct proxy_data* data = (struct proxy_data*) closure;
    if (data->client == NULL) {
        return;
    }
    connection_close(data->client);
    data->client = NULL;
    data->backend = NULL;
    epoll_add_to_free_list(closure);
}

/*
 * This function:
 *   (1) Register client fd and callback functions
 *   (2) Create and register backend fd and callback functions
 */
void handle_client_connection(int client_socket_fd, struct server_endpoint* endpoint)
{
    struct epoll_event_handler* client_connection;
    rsp_log("Creating connection object for incoming connection...");
    client_connection = create_connection(client_socket_fd);

    int backend_socket_fd = connect_to_backend(endpoint->address, endpoint->port);
    struct epoll_event_handler* backend_connection;
    rsp_log("Creating connection object for backend connection (%s:%s)...",
            endpoint->address, endpoint->port);
    backend_connection = create_connection(backend_socket_fd);

    struct proxy_data* proxy = malloc(sizeof(struct proxy_data));
    proxy->client = client_connection;
    proxy->backend = backend_connection;

    struct connection_closure* client_closure = (struct connection_closure*) client_connection->closure;
    client_closure->on_read = on_client_read;
    client_closure->on_read_closure = proxy;
    client_closure->on_close = on_client_close;
    client_closure->on_close_closure = proxy;

    struct connection_closure* backend_closure = (struct connection_closure*) backend_connection->closure;
    backend_closure->on_read = on_backend_read;
    backend_closure->on_read_closure = proxy;
    backend_closure->on_close = on_backend_close;
    backend_closure->on_close_closure = proxy;
}

/* The callback function handling an incoming connection to the load balancer. */
void handle_server_socket_event(struct epoll_event_handler* self, uint32_t events)
{
    struct queue_root* backend_list = (struct queue_root*) self->closure;
    int client_socket_fd;
    struct queue_item* selected_backend;
    struct server_endpoint* backend_endpoint;

    while (1) {
        client_socket_fd = accept(self->fd, NULL, NULL);
        if (client_socket_fd == -1) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                break;
            } else {
                rsp_log_error("Could not accept");
                exit(1);
            }
        }

        selected_backend = dequeue(backend_list);

        if (selected_backend) {
            backend_endpoint = (struct server_endpoint*) selected_backend->data;

            enqueue(backend_list, selected_backend);

            handle_client_connection(client_socket_fd, backend_endpoint);
        } else {
            rsp_log_error("No backend found");
            exit(1);
        }
    }
}


int create_and_bind(const char* server_port_str)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* addrs;
    int getaddrinfo_error;
    getaddrinfo_error = getaddrinfo(NULL, server_port_str, &hints, &addrs);
    if (getaddrinfo_error != 0) {
        rsp_log("Couldn't find local host details: %s", gai_strerror(getaddrinfo_error));
        exit(1);
    }

    int server_socket_fd;
    struct addrinfo* addr_iter;
    for (addr_iter = addrs; addr_iter != NULL; addr_iter = addr_iter->ai_next) {
        server_socket_fd = socket(addr_iter->ai_family,
                                  addr_iter->ai_socktype,
                                  addr_iter->ai_protocol);
        if (server_socket_fd == -1) {
            continue;
        }

        int so_reuseaddr = 1;
        if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr)) != 0) {
            continue;
        }

        if (bind(server_socket_fd,
                 addr_iter->ai_addr,
                 addr_iter->ai_addrlen) == 0)
        {
            break;
        }

        close(server_socket_fd);
    }

    if (addr_iter == NULL) {
        rsp_log("Couldn't bind");
        exit(1);
    }

    freeaddrinfo(addrs);

    return server_socket_fd;
}


struct epoll_event_handler* create_server_socket_handler(char* server_port_str,
                                                         struct queue_root* backend_list)
{

    int server_socket_fd;
    server_socket_fd = create_and_bind(server_port_str);
    make_socket_non_blocking(server_socket_fd);

    /*
     * Mark that this socket will be used to accept incoming connection
     * requests using accept.
     */
    listen(server_socket_fd, MAX_LISTEN_BACKLOG);

    struct epoll_event_handler* result = malloc(sizeof(struct epoll_event_handler));
    result->fd = server_socket_fd;
    result->handle = handle_server_socket_event;
    result->closure = backend_list;

    epoll_add_handler(result, EPOLLIN | EPOLLET);

    return result;
}


