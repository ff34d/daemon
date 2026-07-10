#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#define SOCKET_SERVER_NAME "SOCKET_SERVER"
#define DEFAULT_PORT 8080

typedef struct {
    int port;
} proxy_config_t;

int initialize_socket_server(proxy_config_t *config);

#endif