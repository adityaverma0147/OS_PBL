#include "server.h"
#include <stdio.h>    // For printf()
#include <stdlib.h>   // For EXIT_FAILURE
#include <string.h>   // For strlen()
#include <unistd.h>   // For read(), write(), close()
#include <sys/socket.h> // For accept() and constants like AF_INET, SOCK_STREAM

// Function Prototype: The custom launch function that handles requests.
void launch(struct server *server);

// ====================================================================
// The Launch Function: This is the server's main request loop.
// ====================================================================
void launch(struct server *server)
{
    char buffer[30000];
    int new_socket;
    socklen_t address_length = sizeof(server->address);

    // The server runs indefinitely, waiting for new connections
    while (1) {
        printf("===== WAITING FOR CONNECTION =====\n");

        // 1. Accept an incoming connection
        // This call blocks until a client connects.
        new_socket = accept(
            server->socket,
            (struct sockaddr *)&server->address,
            &address_length
        );

        if (new_socket < 0) {
            perror("Error accepting connection");
            continue; // Continue to the next iteration of the loop
        }

        // 2. Read the client's request into the buffer
        read(new_socket, buffer, 30000);

        // 3. Print the request (for debugging/handling)
        printf("%s\n", buffer);

        // 4. Define and send the HTTP response
        // A simple HTTP/1.1 response must include the status line, a blank line (\r\n\r\n), and the body.
        char *hello = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from C Server!";
        
        write(new_socket, hello, strlen(hello));

        // 5. Close the client-specific socket
        close(new_socket);
    }
}


int main()
{
    // Define the network parameters
    int domain = AF_INET;
    int service = SOCK_STREAM;
    int protocol = 0;
    unsigned long interface = INADDR_ANY; 
    int port = 8080;                     
    int backlog = 10;


    struct server server = server_constructor(
        domain,
        service,
        protocol,
        interface,
        port,
        backlog,
        launch // Pass the function pointer to the custom launch logic
    );


    printf("Server started on port %d...\n", port);
    server.launch(&server); 


    return 0; 
}