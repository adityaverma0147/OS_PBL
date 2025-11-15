#ifndef ROUTES_H
#define ROUTES_H
void handle_hello(int client_socket, const char* query);
void handle_time(int client_socket, const char* query);
#endif
