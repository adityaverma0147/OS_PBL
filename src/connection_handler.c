#include "connection_handler.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>


int accept_connection(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_socket < 0) {
        perror("Accept failed");
        return -1;
    }
    return client_socket;
}

int read_from_client(int client_socket, char* buffer, int size) {
    int bytes_read = read(client_socket, buffer, size - 1);
    if (bytes_read < 0) {
        perror("Read failed");
        return -1;
    }
    buffer[bytes_read] = '\0';
    return bytes_read;
}

int write_to_client(int client_socket, const char* buffer, int size) {
    int bytes_written = write(client_socket, buffer, size);
    if (bytes_written < 0) {
        perror("Write failed");
        return -1;
    }
    return bytes_written;
}

void close_connection(int client_socket) {
    close(client_socket);
}
