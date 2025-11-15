#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

struct server server_constructor(int domain, int service, int protocol, unsigned long interface, int port, int backlog) {
    struct server server;

    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;

    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.socket = socket(domain, service, protocol);
    if (server.socket < 0) { perror("Socket failed"); exit(EXIT_FAILURE); }

    if (bind(server.socket, (struct sockaddr*)&server.address, sizeof(server.address)) < 0) {
        perror("Bind failed"); exit(EXIT_FAILURE);
    }

    if (listen(server.socket, backlog) < 0) { perror("Listen failed"); exit(EXIT_FAILURE); }

    return server;
}
