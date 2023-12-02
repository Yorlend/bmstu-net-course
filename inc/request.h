#pragma once

#define REQUEST_MAX_HEADERS_COUNT 200

typedef enum {
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2_0,
} http_version_t;

const char* http_version_str(http_version_t version);

typedef enum {
    OPTIONS,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    CONNECT,
} request_method_t;

const char* request_method_str(request_method_t method);

typedef enum {
    PARSE_SUCCESS,
    INVALID_METHOD,
    INVALID_URI,
    INVALID_HTTP_VERSION,
    EXPECTED_SP,
    EXPECTED_CRLF,
    INVALID_HEADERS,
    INVALID_BODY,
} parse_status_t;

struct request_header {
    const char* key;
    const char* value;
};

struct request {
    http_version_t http_version;
    request_method_t method;
    const char* uri;
    struct request_header headers[REQUEST_MAX_HEADERS_COUNT];
    const char* body;
};

/**
 * Parse request from buffer (buffer must persist until request is destroyed)
 */
parse_status_t parse_request(/* out */ struct request *request, /* var */ char* buffer);

void receive_request(int client_socket_fd);
