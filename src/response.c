#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#include "config.h"
#include "logger.h"
#include "response.h"

#define HEAD_BUFFER_SIZE 256

struct response create_response(http_version_t version, int status)
{
    struct response response = {
        .version = version,
        .status = status,
        .headers_buffer_size = 0,
        .body_writer = NULL,
        .body_writer_data = NULL
    };

    // add default headers here
    add_header(&response, "Server", SERVER_NAME);

    time_t now = time(NULL);
    struct tm* now_tm = gmtime(&now);

    char asctime_buf[64];
    strftime(asctime_buf, sizeof(asctime_buf), "%Y-%m-%d %H:%M:%S", now_tm);
    add_header(&response, "Date", asctime_buf);

    return response;
}

void set_body_writer(struct response* response, body_writer_fn_t body_writer, void* body_writer_data)
{
    if (response->body_writer)
        LOG_WARNING("body_writer already set. Reseting");
    response->body_writer = body_writer;
    response->body_writer_data = body_writer_data;
}

int add_header(struct response* response, const char* name, const char* value)
{
    size_t header_row_len = strlen(name) + strlen(value) + 4;
    if (response->headers_buffer_size + header_row_len >= HEADER_MAP_BUFFER_SIZE)
    {
        LOG_WARNING("headers buffer is full header_name=%s header_value=%s", name, value);
        return EXIT_FAILURE;
    }

    snprintf(response->headers_buffer + response->headers_buffer_size,
        HEADER_MAP_BUFFER_SIZE - response->headers_buffer_size, "%s: %s\r\n", name, value);
    response->headers_buffer_size += header_row_len;
    return EXIT_SUCCESS;
}

int print_header(struct response* response, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t header_row_len = vsnprintf(NULL, 0, fmt, args) + 2;
    va_end(args);

    if (response->headers_buffer_size + header_row_len >= HEADER_MAP_BUFFER_SIZE)
    {
        LOG_WARNING("headers buffer is full");
        return EXIT_FAILURE;
    }

    va_start(args, fmt);
    size_t written = vsnprintf(response->headers_buffer + response->headers_buffer_size,
        HEADER_MAP_BUFFER_SIZE - response->headers_buffer_size, fmt, args);
    va_end(args);

    snprintf(response->headers_buffer + response->headers_buffer_size + written,
        HEADER_MAP_BUFFER_SIZE - response->headers_buffer_size - written, "\r\n");
    response->headers_buffer_size += header_row_len;
    return EXIT_SUCCESS;
}

static int send_response_head(int client_socket, const struct response* response)
{
    char response_buffer[HEAD_BUFFER_SIZE];
    snprintf(response_buffer, HEAD_BUFFER_SIZE, "%s %d %s\r\n",
        http_version_str(response->version),
        response->status,
        http_status_str(response->status));
    
    ssize_t sent = send(client_socket, response_buffer, strlen(response_buffer), 0);
    if (sent == -1)
    {
        LOG_ERROR("send: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int send_response(int client_socket, const struct response* response)
{
    if (send_response_head(client_socket, response) != EXIT_SUCCESS)
    {
        LOG_ERROR("failed to send response head");
        return EXIT_FAILURE;
    }

    ssize_t sent = send(client_socket, response->headers_buffer, response->headers_buffer_size, 0);
    if (sent == -1)
    {
        LOG_ERROR("failed to send headers: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    if (response->body_writer)
    {
        if (send(client_socket, "\r\n", 2, 0) == -1)
        {
            LOG_ERROR("failed to send body delimiter: %s", strerror(errno));
            return EXIT_FAILURE;
        }

        if (response->body_writer(client_socket, response->body_writer_data) != EXIT_SUCCESS)
        {
            LOG_ERROR("failed to send response body");
            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
}

int simple_body_writer(int client_socket, void* data)
{
    if (send(client_socket, data, strlen(data), 0) == -1)
    {
        LOG_ERROR("failed to send body: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int send_simple_response(int client_socket, http_version_t version, http_status_code_t status, const char* body)
{
    struct response response = create_response(version, status);

    if (body != NULL)
    {
        add_header(&response, "Content-Type", "text/plain");
        print_header(&response, "Content-Length: %zu", strlen(body));

        set_body_writer(&response, simple_body_writer, (char*)body);
    }
    
    return send_response(client_socket, &response);
}
