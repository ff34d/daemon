#include "include/proxy_worker.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "include/logger.h"
#include "include/socket_server.h"
#include "include/socks5.h"

// ======================================================================
// Proxy worker
// ======================================================================
void *proxy_worker(void *arg) {
    proxy_config_t *config = (proxy_config_t *)arg;

    int server_fd = initialize_socket_server(config);
    if (server_fd < 0) return NULL;

    // Main accept loop
    while (1) {
        // Accepting client
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            log_message(LOG_ERROR, PROXY_WORKER_NAME, "Accept client failed");
            continue;
        }

        log_message(LOG_INFO, PROXY_WORKER_NAME, "New client connected");

        int buffer_atype = socks5_handshake(client_fd);
        if (buffer_atype < 0) {
            log_message(
                LOG_ERROR, PROXY_WORKER_NAME, "Socket handshake failed");
            close(client_fd);
            continue;
        }

        char dest_host[256] = {0};
        int dest_port = 0;

        // Read address
        if (socks5_read_address(
                client_fd, sizeof(dest_host), buffer_atype, dest_host) < 0) {
            log_message(
                LOG_ERROR, PROXY_WORKER_NAME, "Request address reading failed");
            close(client_fd);
            continue;
        }

        // Read port
        if (socks5_read_port(client_fd, &dest_port) < 0) {
            log_message(
                LOG_ERROR, PROXY_WORKER_NAME, "Request port reading failed");
            close(client_fd);
            continue;
        }

        printf("\nhost: %s, port: %d\n\n", dest_host, dest_port);

        close(client_fd);
    }

    return NULL;
}
