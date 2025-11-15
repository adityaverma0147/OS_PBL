#include "server.h"
#include "client_handler.h"
#include "http_methods.h"
#include "connection_handler.h"
#include "logger.h"
#include "router.h"
#include "routes.h"
#include "cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>   // for fork, pipe
#include <thread_pool.h>
#include <arpa/inet.h> // for ntohs()

int main() {

    init_cache();
    
    int log_pipe[2];
    if (pipe(log_pipe) < 0) {
        perror("Pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child = Logger Process
        close(log_pipe[1]);  // close write end
        run_logger_process(log_pipe[0]);
        exit(0);
    }

    // Parent = Server
    close(log_pipe[0]); 
    set_log_pipe(log_pipe[1]); 

    init_router();
    register_route("GET", "/hello", handle_hello);
    register_route("GET", "/time", handle_time);

    int domain = AF_INET;
    int service = SOCK_STREAM;
    int protocol = 0;
    unsigned long interface = INADDR_ANY;

    int port = 8080;
    int max_port = 9000;
    int backlog = 10;

    struct server server;

    while (1) {
        server = server_constructor(domain, service, protocol, interface, port, backlog);
        
        if (server.socket >= 0) {
            printf("✅ Server bound to port %d\n", port);
            break;
        }

        printf("⚠️ Port %d busy, trying %d...\n", port, port + 1);
        port++;

        if (port > max_port) {
            fprintf(stderr, "❌ No free port found between 8080-9000\n");
            exit(1);
        }
    }

    char logmsg[64];
    snprintf(logmsg, sizeof(logmsg), "Server Started on port %d", port);
    send_log_ipc(logmsg);

    ThreadPool pool;
    thread_pool_init(&pool, 8, 100);

    while (1) {
        int client_socket = accept_connection(server.socket);
        if (client_socket < 0) continue;
        thread_pool_add(&pool, client_socket);
    }

    thread_pool_destroy(&pool);
    close_logger();
    return 0;
}
