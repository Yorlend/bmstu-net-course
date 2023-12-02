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
#include "response.h"

#define FILEPATH_BUFFER_SIZE 512
#define RESPONSE_BUFFER_SIZE 4096

static int text_file_writer(int client_socket, void* data)
{
    return send_text_file(client_socket, (const char*)data);
}

static void respond_text_file(int client_socket, const char* path)
{
    if (!file_exists(path))
        send_simple_response(client_socket, HTTP_1_1, HTTP_STATUS_NOT_FOUND, NULL);
    else
    {
        struct response response = create_response(HTTP_1_1, HTTP_STATUS_OK);
        set_body_writer(&response, text_file_writer, (char*)path);
        
        if (send_response(client_socket, &response) != EXIT_SUCCESS)
            LOG_ERROR("failed to send text file at path=%s", path);
    }
}

static void respond_file_head(int client_socket, const char* path)
{
    if (!file_exists(path))
        send_simple_response(client_socket, HTTP_1_1, HTTP_STATUS_NOT_FOUND, NULL);
    else
    {
        struct response response = create_response(HTTP_1_1, HTTP_STATUS_OK);
        print_header(&response, "Content-Length", text_file_length(path));
        
        if (send_response(client_socket, &response) != EXIT_SUCCESS)
            LOG_ERROR("failed to send head of file at path=%s", path);
    }
}

void handle_request(int client_socket, struct request request)
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
        send_simple_response(client_socket, HTTP_1_1, HTTP_STATUS_METHOD_NOT_ALLOWED, NULL);
    }
}
