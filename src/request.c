#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "fs.h"
#include "http_status_codes.h"
#include "request.h"
#include "security.h"

#define REQUEST_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 4096
#define FILEPATH_BUFFER_SIZE 512

static void respond_error(int client_socket, int status, const char* message)
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
    char response_buffer[RESPONSE_BUFFER_SIZE];
    snprintf(response_buffer, RESPONSE_BUFFER_SIZE,
        "HTTP/1.1 %d\r\n"
        "Content-Length: %d\r\n"
        "\r\n", HTTP_STATUS_OK, text_file_length(path));
    
    send(client_socket, response_buffer, strlen(response_buffer), 0);
    if (send_text_file(client_socket, path) != EXIT_SUCCESS)
        fprintf(stderr, "failed to send text file at %s\n", path);
}

void handle_request(int client_socket_fd)
{
    char request_buffer[REQUEST_BUFFER_SIZE];
    struct request request;
    parse_status_t parse_status;

    int bytes_received = recv(client_socket_fd, request_buffer, REQUEST_BUFFER_SIZE, 0);
    if (bytes_received == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    else if (bytes_received == 0)
    {
        fprintf(stderr, "received 0 bytes\n");
        respond_error(client_socket_fd, HTTP_STATUS_BAD_REQUEST, "bad request");
    }
    else if (bytes_received == REQUEST_BUFFER_SIZE)
    {
        fprintf(stderr, "received too many bytes\n");
        respond_error(client_socket_fd, HTTP_STATUS_BAD_REQUEST, "bad request");        
    }
    else if ((parse_status = parse_request(&request, request_buffer)) != PARSE_SUCCESS)
    {
        fprintf(stderr, "parse request failed with code: %d\n", parse_status);
        respond_error(client_socket_fd, HTTP_STATUS_BAD_REQUEST, "bad request");
    }
    else
    {
        char filepath_buffer[FILEPATH_BUFFER_SIZE];
        getcwd(filepath_buffer, FILEPATH_BUFFER_SIZE);
        join_paths_secure(filepath_buffer, FILEPATH_BUFFER_SIZE, filepath_buffer, request.uri);

        respond_text_file(client_socket_fd, filepath_buffer);
    }

    close(client_socket_fd);
}
