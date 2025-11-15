#ifndef HTTP_METHODS_H
#define HTTP_METHODS_H

#include <stddef.h>

struct http_request {
    char method[8];
    char path[256];
    char query[256];
    char version[16];
};

char* generate_response(const char* status, const char* content_type, const char* body);

// Parses the raw request and fills an http_request struct
int parse_http_request(const char* raw, struct http_request* req);
char* serve_static_file(const char* path, const char* root_dir);
const char* get_mime_type(const char* path);
// Function: Parses the HTTP request and returns appropriate response
// Input: request buffer (char*), Output: dynamically allocated response string
char* handle_http_request(const char* request);

#endif