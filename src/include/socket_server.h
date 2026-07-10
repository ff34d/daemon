#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#define SOCKET_SERVER_NAME "SOCKET_SERVER"
#define DEFAULT_PORT 8080
#define DEFAULT_BUFFER_SIZE 1024

typedef struct {
    int port;
    int buffer_size;
} proxy_config_t;

int initialize_socket_server(proxy_config_t *config);

#endif