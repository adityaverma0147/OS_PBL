#ifndef ROUTER_H
#define ROUTER_H

typedef void (*RouteHandler)(int client_socket, const char* query);

typedef struct {
    char method[8];
    char path[128];
    RouteHandler handler;
} Route;

void init_router();
void register_route(const char* method, const char* path, RouteHandler handler);
RouteHandler find_route(const char* method, const char* path);

#endif
