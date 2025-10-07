#include "server.h"
#include <stdio.h>    // For perror()
#include <stdlib.h>   // For exit()
#include <unistd.h>   // For close() (though not in constructor, good to include)

// Implementation of the server constructor function
struct server server_constructor(
    int domain,
    int service,
    int protocol,
    unsigned long interface,
    int port,
    int backlog,
    void (*launch)(struct server *server)
)
{
    // 1. Instantiate a server object
    struct server server;

    // 2. Assign configuration parameters to the server struct members
    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interface = interface;
    server.port = port;
    server.backlog = backlog;
    server.launch = launch; // Assign the custom launch function pointer

    // 3. Configure the server's network address structure (sockaddr_in)
    // The sin_family and sin_port fields must be in network byte order (Big Endian)
    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    // 4. Create the socket
    server.socket = socket(domain, service, protocol);

    // Error Check: Ensure the socket was created successfully
    if (server.socket == 0) {
        perror("Failed to connect to socket");
        exit(EXIT_FAILURE);
    }

    // 5. Bind the socket to the specified address and port
    // The address must be cast to (struct sockaddr *) for the bind function
    if (bind(
            server.socket, 
            (struct sockaddr *)&server.address, 
            sizeof(server.address)
        ) < 0) {
        
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // 6. Start listening for incoming connections
    if (listen(server.socket, backlog) < 0) {
        perror("Failed to start listening");
        exit(EXIT_FAILURE);
    }

    // 7. Return the initialized server object
    return server;
}