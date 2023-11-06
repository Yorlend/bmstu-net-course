#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http_status_codes.h"
#include "request.h"

#define REQUEST_BUFFER_SIZE 4096
#define RESPONSE_BUFFER_SIZE 4096

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

void handle_request(int client_socket_fd)
{
    char request_buffer[REQUEST_BUFFER_SIZE];
    struct request request;

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
    else
    {
        parse_status_t parse_status = parse_request(&request, request_buffer);
        if (parse_status != PARSE_SUCCESS)
            respond_error(client_socket_fd, parse_status, "bad request");
        else
            respond_error(client_socket_fd, HTTP_STATUS_OK, "ok");
    }

    close(client_socket_fd);
}
