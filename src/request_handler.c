#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "logger.h"
#include "http_status_codes.h"
#include "fs.h"
#include "security.h"
#include "request_handler.h"

#define FILEPATH_BUFFER_SIZE 512
#define RESPONSE_BUFFER_SIZE 4096

void respond_error(int client_socket, int status, const char* message)
{
    char response_buffer[RESPONSE_BUFFER_SIZE];
    snprintf(response_buffer, RESPONSE_BUFFER_SIZE,
        "HTTP/1.1 %d %s\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s", status, message, strlen(message), message);
    
    send(client_socket, response_buffer, strlen(response_buffer), 0);
}

static void respond_text_file(int client_socket, const char* path)
{
    if (!file_exists(path))
        respond_error(client_socket, HTTP_STATUS_NOT_FOUND, "Not Found");
    else
    {
        char response_buffer[RESPONSE_BUFFER_SIZE];
        snprintf(response_buffer, RESPONSE_BUFFER_SIZE,
            "HTTP/1.1 %d\r\n"
            "\r\n", HTTP_STATUS_OK);
        
        send(client_socket, response_buffer, strlen(response_buffer), 0);
        if (send_text_file(client_socket, path) != EXIT_SUCCESS)
            LOG_ERROR("failed to send text file path=%s", path);
    }
}

static void respond_file_head(int client_socket, const char* path)
{
    if (!file_exists(path))
        respond_error(client_socket, HTTP_STATUS_NOT_FOUND, "Not Found");
    else
    {
        char response_buffer[RESPONSE_BUFFER_SIZE];
        snprintf(response_buffer, RESPONSE_BUFFER_SIZE,
            "HTTP/1.1 %d\r\n"
            "Content-Length: %d\r\n"
            "\r\n", HTTP_STATUS_OK, text_file_length(path));
        
        if (send(client_socket, response_buffer, strlen(response_buffer), 0) == -1)
            LOG_ERROR("send: %s", strerror(errno));
    }
}

int handle_request(int client_socket, struct request request)
{
    LOG_INFO("%s %s %s", request_method_str(request.method), request.uri, http_version_str(request.http_version));

    char filepath_buffer[FILEPATH_BUFFER_SIZE];
    getcwd(filepath_buffer, FILEPATH_BUFFER_SIZE);
    join_paths_secure(filepath_buffer, FILEPATH_BUFFER_SIZE, filepath_buffer, request.uri);
    switch (request.method)
    {
    case GET:
        respond_text_file(client_socket, filepath_buffer);
        break;
    
    case HEAD:
        respond_file_head(client_socket, filepath_buffer);
        break;
    
    default:
        LOG_INFO("unsupported method: %s", request_method_str(request.method));
        respond_error(client_socket, HTTP_STATUS_METHOD_NOT_ALLOWED, "method not allowed");
    }

    return EXIT_SUCCESS;
}
