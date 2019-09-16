#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <string.h>

#include "epollinterface.h"
#include "logging.h"
#include "server_socket.h"
#include "luautils.h"
#include "queue.h"

void read_backends_config( lua_State* L, struct queue_root* list) {
  int i, backend_count = 0;

  lua_intexpr( L, "#backends", &backend_count ) ;

  for ( i=0; i<backend_count; ++i ) {
    char expr[64] = "" ;
    sprintf( expr, "backends[%d]", i+1 );
    char* backend_str = strdup(lua_stringexpr( L, expr, 0 ));

    struct queue_item* backend_entry = (struct queue_item*)malloc(sizeof(struct queue_item));
    struct server_endpoint* backend_endpoint = (struct server_endpoint*)
                malloc(sizeof(struct server_endpoint));

    char* token;

    if ((token = strsep(&backend_str, ":")) != NULL)
      strncpy(backend_endpoint->address, token, MAX_ADDR_BUFF);

    if ((token = strsep(&backend_str, ":")) != NULL)
      strncpy(backend_endpoint->port, token, MAX_PORT_BUFF);

    backend_entry->data = backend_endpoint;
    enqueue(list, backend_entry);

    free(backend_str);
  }
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s <config_file>\n",
                argv[0]);
        exit(1);
    }

    lua_State *L = lua_open();
    if (luaL_dofile(L, argv[1]) != 0) {
        fprintf(stderr, "Error parsing config file: %s\n", lua_tostring(L, -1));
        exit(1);
    }

    char* server_port_str = strdup(lua_stringexpr( L, "listenPort", 0 ));

    struct queue_root* backend_list = (struct queue_root*)malloc(sizeof(struct queue_root));

    init_queue(backend_list);

    read_backends_config(L, backend_list);

    lua_close(L);

    /*
     * A SIGPIPE is sent to a process if it tried to write to a socket
     * that had been shutdown for writing or isn't connected anymore.
     * To avoid that the program ends in this case, just ignore this signal.
     */
    signal(SIGPIPE, SIG_IGN);

    epoll_init();

    /*
     * Create a socket listening to incoming connections and
     * register it to epoll.
     */
    create_server_socket_handler(server_port_str, backend_list);

    rsp_log("Started. Listening on port %s.", server_port_str);

    /*
     * A never-ending loop: wait for an epoll event on the registred fds and
     * call the appropriate handler.
     */
    epoll_do_reactor_loop();

    return 0;
}

