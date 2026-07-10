#include "include/socket_server.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "include/logger.h"

// ============================================================
// Initialization for socket, binding and listening.
// ============================================================
int initialize_socket_server(proxy_config_t *config) {
    int port = DEFAULT_PORT;

    if (config && config->port != 0) port = config->port;

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        log_message(LOG_ERROR, SOCKET_SERVER_NAME, "Socket create failed");
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    log_message(LOG_INFO, SOCKET_SERVER_NAME, "Socket created");

    // Create socket address
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    log_message(LOG_INFO, SOCKET_SERVER_NAME, "Socket address created");

    // Binding socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        log_message(LOG_ERROR, SOCKET_SERVER_NAME, "Binding socket failed");
        close(server_fd);
        return -1;
    }

    // Listening socket
    if (listen(server_fd, 1) < 0) {
        log_message(LOG_ERROR, SOCKET_SERVER_NAME, "Listening socket failed");
        close(server_fd);
        return -1;
    }

    log_message(LOG_INFO, SOCKET_SERVER_NAME, "Listening socket");
    printf("Listening socket on 127.0.0.1:%d...\n", port);

    return server_fd;
}
