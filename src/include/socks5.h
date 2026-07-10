#ifndef SOCKS5_H
#define SOCKS5_H

#define SOCKS5_NAME "SOCKS5"

#define SOCKS5_VERSION 5
#define SOCKS5_CMD_CONNECT 1

#define SOCKS5_METHOD_NO_AUTH 0x00

#define SOCKS5_ATYP_IPV4 0x01
#define SOCKS5_ATYP_DOMAIN 0x03
#define SOCKS5_ATYP_IPV6 0x04

int socks5_handshake(int client_fd);
int socks5_read_address(int client_fd, int size, int atyp, char *dest_host);
int socks5_read_port(int client_fd, int *dest_port);

#endif
