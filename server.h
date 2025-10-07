#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h> // For socket(), bind(), listen(), accept()
#include <netinet/in.h> // For struct sockaddr_in and network byte order functions

// Forward declaration of struct server so it can be used in the function pointer
struct server;

// Definition of the server structure (our "server object")
struct server {
    // Network configuration properties
    int domain;
    int service;
    int protocol;
    unsigned long interface;
    int port;
    int backlog;

    // Network components
    struct sockaddr_in address;
    int socket; // The socket file descriptor

    // Function pointer for the launch/request handling logic
    // This is the function the user defines in 'test.c'
    void (*launch)(struct server *server);
};

// Function prototype for the server constructor
// This function initializes the struct, creates the socket, binds it, and starts listening.
struct server server_constructor(
    int domain,
    int service,
    int protocol,
    unsigned long interface,
    int port,
    int backlog,
    void (*launch)(struct server *server)
);

#endif // SERVER_H