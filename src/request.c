#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "http_status_codes.h"
#include "request.h"
#include "response.h"
#include "request_handler.h"

#define REQUEST_BUFFER_SIZE 4096

void receive_request(int client_socket_fd)
{
    char request_buffer[REQUEST_BUFFER_SIZE];
    struct request request;
    parse_status_t parse_status;

    int bytes_received = recv(client_socket_fd, request_buffer, REQUEST_BUFFER_SIZE, 0);
    if (bytes_received == -1)
    {
        LOG_ERROR("recv: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if (bytes_received == 0)
    {
        LOG_WARNING("received 0 bytes");
        send_simple_response(client_socket_fd, HTTP_1_1, HTTP_STATUS_BAD_REQUEST, NULL);
    }
    else if (bytes_received == REQUEST_BUFFER_SIZE)
    {
        LOG_WARNING("received too many bytes");
        send_simple_response(client_socket_fd, HTTP_1_1, HTTP_STATUS_BAD_REQUEST, NULL);        
    }
    else if ((parse_status = parse_request(&request, request_buffer)) != PARSE_SUCCESS)
    {
        LOG_WARNING("parse request failed with code: %d", parse_status);
        send_simple_response(client_socket_fd, HTTP_1_1, HTTP_STATUS_BAD_REQUEST, NULL);
    }
    else
    {
        handle_request(client_socket_fd, request);
    }

    close(client_socket_fd);
}
