#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

struct server {
    int domain;
    int service;
    int protocol;
    unsigned long interface;
    int port;
    int backlog;
    int socket;
    struct sockaddr_in address;
};

struct server server_constructor(int domain, int service, int protocol, unsigned long interface, int port, int backlog);

#endif
