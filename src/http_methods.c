#include "http_methods.h"
#include "router.h"
#include "cache.h"  

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Helper function: Generate HTTP response dynamically
// Returns a malloc'ed buffer that the caller must free.
char* generate_response(const char* status, const char* content_type, const char* body) {
    size_t size = strlen(status) + strlen(content_type) + strlen(body) + 64; // extra for formatting
    char* response = (char*)malloc(size);
    if (!response) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }
    snprintf(response, size, "%s\r\nContent-Type: %s\r\n\r\n%s", status, content_type, body);
    return response;
}

// request-type
int parse_http_request(const char* raw, struct http_request* req) {
    if (!raw || !req) return -1;

    // Extract first line: e.g. "GET /path?query HTTP/1.1"
    const char* line_end = strstr(raw, "\r\n");
    if (!line_end) return -1;

    char first_line[512];
    size_t len = line_end - raw;
    if (len >= sizeof(first_line)) len = sizeof(first_line) - 1;
    strncpy(first_line, raw, len);
    first_line[len] = '\0';

    // Parse method, path+query, version
    // Ensure req->method, req->path and req->version have appropriate sizes in header
    sscanf(first_line, "%7s %255s %15s", req->method, req->path, req->version);

    // Split path and query
    char* qmark = strchr(req->path, '?');
    if (qmark) {
        // ensure req->query buffer is big enough in header definition
        strcpy(req->query, qmark + 1);
        *qmark = '\0';
    } else {
        req->query[0] = '\0';
    }

    return 0;
}

// Detect MIME type from extension
const char* get_mime_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".json") == 0) return "application/json";
    return "text/plain";
}

char* serve_static_file(const char* path, const char* root_dir) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", root_dir, path);

    // Prevent directory traversal
    if (strstr(path, "..")) {
        return generate_response("HTTP/1.1 403 Forbidden", "text/plain", "Access Denied");
    }

    size_t cached_size;
    char* cached = get_from_cache(full_path, &cached_size);
    if (cached) {
        const char* mime = get_mime_type(full_path);
        char header[128];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mime);

        char* response = malloc(strlen(header) + cached_size + 1);
        memcpy(response, header, strlen(header));
        memcpy(response + strlen(header), cached, cached_size);
        response[strlen(header) + cached_size] = '\0';
        free(cached);
        return response;
    }

    FILE* file = fopen(full_path, "rb");
    if (!file) {
        return generate_response("HTTP/1.1 404 Not Found", "text/plain", "File Not Found");
    }

    // Get file size
    struct stat st;
    if (stat(full_path, &st) != 0) {
        fclose(file);
        return generate_response("HTTP/1.1 500 Internal Server Error", "text/plain", "Stat Failed");
    }
    size_t size = (size_t)st.st_size;

    // Read file into memory
    char* body = malloc(size + 1);
    if (!body) {
        fclose(file);
        return generate_response("HTTP/1.1 500 Internal Server Error", "text/plain", "Memory Error");
    }
    size_t read_n = fread(body, 1, size, file);
    if (read_n != size) {
        // reading error or truncated file
        free(body);
        fclose(file);
        return generate_response("HTTP/1.1 500 Internal Server Error", "text/plain", "File Read Error");
    }
    body[size] = '\0';

    put_in_cache(full_path, body, size);
    // fclose(file);

    

    const char* mime = get_mime_type(full_path);
    char header[128];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mime);

    // Combine header + body
    size_t header_len = strlen(header);
    char* response = malloc(header_len + size + 1);
    if (!response) {
        free(body);
        return generate_response("HTTP/1.1 500 Internal Server Error", "text/plain", "Memory Error");
    }
    memcpy(response, header, header_len);
    memcpy(response + header_len, body, size);
    response[header_len + size] = '\0';

    free(body);
    return response;
}

// handle_http_request:
// - Returns a malloc'ed response string (must be freed by caller) for static and
//   non-dynamic handlers.
// - Returns NULL when the request should be handled directly by a dynamic route
//   handler (i.e., the caller will write to socket).
char* handle_http_request(const char* request) {
    if (!request || strlen(request) < 4) {
        return generate_response("HTTP/1.1 400 Bad Request", "text/plain", "Invalid HTTP request");
    }

    struct http_request req;
    if (parse_http_request(request, &req) != 0) {
        return generate_response("HTTP/1.1 400 Bad Request", "text/plain", "Invalid HTTP request");
    }

    printf("Method: %s | Path: %s | Query: %s | Version: %s\n",
           req.method, req.path, req.query[0] ? req.query : "(none)", req.version);

    if (strcmp(req.method, "GET") == 0) {
        // NOTE: Do NOT call route handlers from here. Caller (client_handler)
        // is responsible for finding & calling dynamic route handlers because
        // it has the real socket.
        // Serve static files from /public
        const char* path = strcmp(req.path, "/") == 0 ? "/index.html" : req.path;
        return serve_static_file(path, "public");
    } 
    else if (strcmp(req.method, "POST") == 0) {
        return generate_response("HTTP/1.1 200 OK", "application/json", "{\"message\":\"POST request processed\"}");
    } 
    else if (strcmp(req.method, "PUT") == 0) {
        return generate_response("HTTP/1.1 200 OK", "text/plain", "Handled PUT request");
    } 
    else if (strcmp(req.method, "DELETE") == 0) {
        return generate_response("HTTP/1.1 200 OK", "text/plain", "Handled DELETE request");
    } 
    else {
        return generate_response("HTTP/1.1 405 Method Not Allowed", "text/plain", "HTTP Method not supported");
    }
}
