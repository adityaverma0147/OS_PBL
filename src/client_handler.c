#include "client_handler.h"
#include "http_methods.h"
#include "logger.h"
#include "router.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[30000];
    int read_bytes = read(client_socket, buffer, sizeof(buffer) - 1);
    if (read_bytes < 0) {
        perror("Read failed");
        close(client_socket);
        return NULL;
    }
    if (read_bytes == 0) {
        // client closed connection
        close(client_socket);
        return NULL;
    }
    buffer[read_bytes] = '\0';

    // Parse the request so we can log method/path
    struct http_request req;
    int parsed = parse_http_request(buffer, &req);

    char* response = NULL;
    int status = 0;
    int should_free_response = 0; // explicit ownership flag

    if (parsed == 0) {
        // First, check if this path matches a dynamic route
        RouteHandler handler = find_route(req.method, req.path);
        if (handler) {
            // Handler writes directly to socket and manages its own allocations
            handler(client_socket, req.query);
            status = 200;
            response = NULL;  // Nothing to free, handler wrote directly
            should_free_response = 0;
        } else {
            // Otherwise, handle static or built-in methods (caller owns returned buffer)
            response = handle_http_request(buffer);
            // handle_http_request() returns a malloc'ed buffer on success, or NULL.
            should_free_response = response ? 1 : 0;
        }
    } else {
        // Invalid HTTP request
        response = generate_response("HTTP/1.1 400 Bad Request", "text/plain", "Malformed Request");
        should_free_response = 1;
    }

    // If a response was returned, send it and free if owned
    if (response) {
        ssize_t wrote = write(client_socket, response, strlen(response));
        (void)wrote; // ignore write errors for now or optionally log them

        if (strncmp(response, "HTTP/1.1 200", 12) == 0) status = 200;
        else if (strncmp(response, "HTTP/1.1 400", 12) == 0) status = 400;
        else if (strncmp(response, "HTTP/1.1 404", 12) == 0) status = 404;
        else if (strncmp(response, "HTTP/1.1 500", 12) == 0) status = 500;
        else status = 0;

        if (should_free_response) {
            free(response);
            response = NULL;
        }
    }


    char logbuf[256];
    snprintf(logbuf, sizeof(logbuf), "%s %s %d",
    parsed == 0 ? req.method : "INVALID",
    parsed == 0 ? req.path   : "(none)",
    status);
    send_log_ipc(logbuf);

    // // Log the request neatly
    // log_request(
    //     parsed == 0 ? req.method : "INVALID",
    //     parsed == 0 ? req.path : "(none)",
    //     status
    // );

    close(client_socket);
    return NULL;
}

void create_client_thread(int client_socket) {
    pthread_t tid;
    int* pclient = malloc(sizeof(int));
    if (!pclient) {
        perror("malloc");
        close(client_socket);
        return;
    }
    *pclient = client_socket;
    pthread_create(&tid, NULL, handle_client, pclient);
    pthread_detach(tid);
}
