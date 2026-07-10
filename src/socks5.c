#include "include/socks5.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "include/logger.h"

// TODO: Add global buffer for request of handshake

// ======================================================================
// SOCKS5 Handshake
// ======================================================================
int socks5_handshake(int client_fd) {
    unsigned char buffer[258];
    ssize_t n;

    // Read greeting
    n = recv(client_fd, buffer, 2, 0);
    if (n < 2 || buffer[0] != SOCKS5_VERSION) {
        log_message(LOG_ERROR, SOCKS5_NAME, "Bad SOCKS5 greeting");
        return -1;
    }

    // Check methods
    int nmethods = buffer[1];

    n = recv(client_fd, buffer, nmethods, 0);
    if (n != nmethods) {
        log_message(LOG_ERROR, SOCKS5_NAME, "Failed to read methods");
        return -1;
    }

    // Check for no auth required
    int no_auth = 0;

    for (int i = 0; i < nmethods; i++) {
        if (buffer[i] == SOCKS5_METHOD_NO_AUTH) {  // 0x00 - no auth key
            no_auth = 1;
            break;
        }
    }

    if (!no_auth) {
        // 5 - socket version, 0xFF - no acceptable methods
        unsigned char reply[] = {5, 0xFF};
        send(client_fd, reply, sizeof(reply), 0);
        return -1;
    }

    // Send method choice: 0x00 - no auth
    unsigned char method_choice[] = {5, SOCKS5_METHOD_NO_AUTH};
    send(client_fd, method_choice, sizeof(method_choice), 0);

    // Validate request
    n = recv(client_fd, buffer, 4, 0);

    // buffer[5] - Socket version, buffer[1] - Command (1 = Connect)
    if (n < 4 || buffer[0] != 5 || buffer[1] != SOCKS5_CMD_CONNECT) {
        log_message(LOG_ERROR, SOCKS5_NAME, "Bad SOCKS5 request");
        return -1;
    }

    return buffer[3];
}

// ======================================================================
// SOCKS5 Reading address
// ======================================================================
int socks5_read_address(int client_fd, int size, int atyp, char *dest_host) {
    ssize_t n;
    unsigned char buffer[258];
    int read_failed = 0;

    // Read address
    switch (atyp) {
        case SOCKS5_ATYP_IPV4: {  // IPv4
            n = recv(client_fd, buffer, 4, 0);
            if (n < 4) {
                read_failed = -1;
                break;
            }

            snprintf(
                dest_host,
                size,
                "%d.%d.%d.%d",
                buffer[0],
                buffer[1],
                buffer[2],
                buffer[3]);

            break;
        }
        case SOCKS5_ATYP_DOMAIN: {  // Domain name
            unsigned char len;

            n = recv(client_fd, &len, 1, 0);
            if (n < 1) {
                read_failed = -1;
                break;
            }

            n = recv(client_fd, dest_host, len, 0);
            if (n < len) {
                read_failed = -1;
                break;
            }

            dest_host[len] = '\0';
            break;
        }
        case SOCKS5_ATYP_IPV6: {  // IPv6
            unsigned char ipv6[16];

            n = recv(client_fd, ipv6, 16, 0);

            if (n < 16) {
                read_failed = -1;
                break;
            }

            struct in6_addr v6addr;
            memcpy(&v6addr, ipv6, 16);
            inet_ntop(AF_INET6, &v6addr, dest_host, size);
            break;
        }
        default: {
            log_message(LOG_ERROR, SOCKS5_NAME, "Unsupported address type");
            read_failed = -1;
            break;
        }
    }

    return read_failed;
}

// ======================================================================
// SOCKS5 Reading port
// ======================================================================
int socks5_read_port(int client_fd, int *dest_port) {
    unsigned char port_bytes[2];
    ssize_t n;

    n = recv(client_fd, port_bytes, 2, 0);
    if (n < 2) {
        log_message(LOG_ERROR, SOCKS5_NAME, "Read port failed");
        return -1;
    }

    *dest_port = (port_bytes[0] << 8) | port_bytes[1];
    return 0;
}

// ======================================================================
// SOCKS5 Connect to remote server
// ======================================================================
int socks5_connect(int client_fd, char *dest_host, int dest_port) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;  // Allow both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;

    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%d", dest_port);

    if (getaddrinfo(dest_host, port_str, &hints, &res) != 0) {
        log_message(
            LOG_ERROR, SOCKS5_NAME, "Getaddrinfo failed for %s", dest_host);
        // Send SOCKS5 error reply (host unreachable)
        unsigned char err_reply[] = {5, 4, 0, 1, 0, 0, 0, 0, 0, 0};
        send(client_fd, err_reply, sizeof(err_reply), 0);
        close(client_fd);
        return -1;
    }

    int remote_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (remote_fd < 0) {
        freeaddrinfo(res);
        // Send SOCKS5 error reply (general failure)
        unsigned char err_reply[] = {5, 1, 0, 1, 0, 0, 0, 0, 0, 0};
        send(client_fd, err_reply, sizeof(err_reply), 0);
        close(client_fd);
        return -1;
    }

    if (connect(remote_fd, res->ai_addr, res->ai_addrlen) < 0) {
        freeaddrinfo(res);
        close(remote_fd);
        // Send SOCKS5 error reply (connection refused)
        unsigned char err_reply[] = {5, 5, 0, 1, 0, 0, 0, 0, 0, 0};
        send(client_fd, err_reply, sizeof(err_reply), 0);
        close(client_fd);
        return -1;
    }

    // Free address list
    freeaddrinfo(res);

    // Send success reply
    unsigned char success_reply[] = {5, 0, 0, 1, 0, 0, 0, 0, 0, 0};
    send(client_fd, success_reply, sizeof(success_reply), 0);

    log_message(
        LOG_INFO,
        SOCKS5_NAME,
        "Tunnel established with %s:%d",
        dest_host,
        dest_port);

    return remote_fd;
}