#include "router.h"
#include "http_methods.h"
#include "connection_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void handle_hello(int client_socket, const char* query) {
    const char* body = "Hello from C router!";
    char* response = generate_response("HTTP/1.1 200 OK", "text/plain", body);
    write_to_client(client_socket, response, strlen(response));
    free(response);
}

void handle_time(int client_socket, const char* query) {
    char body[128];
    time_t now = time(NULL);
    snprintf(body, sizeof(body), "Current server time: %s", ctime(&now));
    char* response = generate_response("HTTP/1.1 200 OK", "text/plain", body);
    write_to_client(client_socket, response, strlen(response));
    free(response);
}
