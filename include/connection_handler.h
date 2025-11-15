#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

int accept_connection(int server_socket);
int read_from_client(int client_socket, char* buffer, int size);
int write_to_client(int client_socket, const char* buffer, int size);
void close_connection(int client_socket);

#endif
