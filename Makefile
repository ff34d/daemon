CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = daemon

OBJ = src/main.o src/logger.o src/socks5.o src/socket_server.o src/proxy_worker.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -pthread

%.0: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
