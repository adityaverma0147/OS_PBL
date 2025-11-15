#include "router.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_ROUTES 100

static Route routes[MAX_ROUTES];
static int route_count = 0;

void init_router() {
    route_count = 0;
}

void register_route(const char* method, const char* path, RouteHandler handler) {
    if (route_count >= MAX_ROUTES) {
        fprintf(stderr, "Route limit reached!\n");
        return;
    }
    strncpy(routes[route_count].method, method, sizeof(routes[route_count].method) - 1);
    strncpy(routes[route_count].path, path, sizeof(routes[route_count].path) - 1);
    routes[route_count].handler = handler;
    route_count++;
}

RouteHandler find_route(const char* method, const char* path) {
    for (int i = 0; i < route_count; i++) {
        if (strcmp(routes[i].method, method) == 0 &&
            strcmp(routes[i].path, path) == 0) {
            return routes[i].handler;
        }
    }
    return NULL;
}
