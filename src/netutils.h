#ifndef RSLB_NET_UTILS_H
#define RSLB_NET_UTILS_H

extern void make_socket_non_blocking(int socket_fd);
extern int connect_to_backend(char* backend_host, char* backend_port_str);

#endif