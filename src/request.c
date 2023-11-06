#include <pthread.h>
#include <sys/types.h>

#include "request.h"

void handle_request(int client_socket_fd)
{
    pid_t tid = gettid();
    printf("handling request with fd = %d in thread = %d\n", client_socket_fd, tid);

    close(client_socket_fd);
}
