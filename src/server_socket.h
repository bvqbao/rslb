#ifndef RSLB_SERVER_SOCKET_H
#define RSLB_SERVER_SOCKET_H

#include "queue.h"

#define MAX_ADDR_BUFF   250
#define MAX_PORT_BUFF   10

struct server_endpoint {
    char address[MAX_ADDR_BUFF];
    char port[MAX_PORT_BUFF];
};

extern struct epoll_event_handler* create_server_socket_handler(char* server_port_str,
                                                                struct queue_root* backend_list);

#endif
