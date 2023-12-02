#pragma once

#include "request.h"
#include "http_status_codes.h"

#define HEADER_MAP_BUFFER_SIZE 4096
#define HEADER_MAP_MAX_HEADERS 64

typedef int (*body_writer_fn_t)(int client_socket, void* data);

struct response {
    http_version_t version;
    http_status_code_t status;
    char headers_buffer[HEADER_MAP_BUFFER_SIZE];
    int headers_buffer_size;
    body_writer_fn_t body_writer;
    void* body_writer_data;
};

struct response create_response(http_version_t version, int status);

void set_body_writer(struct response* response, body_writer_fn_t body_writer, void* body_writer_data);

int add_header(struct response* response, const char* name, const char* value);

/**
 * Adds custom header to response (CRLF inserted automatically)
 */
int print_header(struct response* response, const char* fmt, ...);

int send_response(int client_socket, const struct response* response);

/**
 * Sends simple response to client.
 * If body is NULL, body is not sent.
 * Content-Type is set to "text/plain".
 * Content-Length is set to length of body if present.
 */
int send_simple_response(int client_socket, http_version_t version, http_status_code_t status, const char* body);
