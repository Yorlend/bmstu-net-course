#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "server.h"
#include "request_queue.h"
#include "thread_pool.h"

#define QUEUE_CAPACITY 10
#define NUM_WORKER_THREADS 4

static struct
{
    struct sockaddr_in addr;
    int queue_capacity;
    int num_worker_threads;
    bool running;
} server;

static void interrupt_handler(int sig)
{
    server.running = false;
    LOG_INFO("server stopped");
}

void init_server(const char* ip_address, int port)
{
    server.addr.sin_family = AF_INET;
    server.addr.sin_addr.s_addr = inet_addr(ip_address);
    server.addr.sin_port = htons(port);
    server.queue_capacity = QUEUE_CAPACITY;
    server.num_worker_threads = NUM_WORKER_THREADS;
    server.running = false;
}

int run_server(void)
{
    // create socket and bind it to address
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1) {
        LOG_ERROR("socket: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        LOG_ERROR("setsockopt: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    if (bind(server_socket_fd, (struct sockaddr*)&server.addr, sizeof(server.addr)) == -1) {
        LOG_ERROR("bind: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    if (listen(server_socket_fd, server.queue_capacity) == -1) {
        LOG_ERROR("listen: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    struct thread_pool* thread_pool = create_thread_pool(server.num_worker_threads);
    if (thread_pool == NULL) {
        LOG_ERROR("create_thread_pool: %s", strerror(errno));
        close(server_socket_fd);
        return EXIT_FAILURE;
    }

    server.running = true;
    __sighandler_t old_handler = signal(SIGINT, interrupt_handler);

    LOG_INFO("server started at %s:%d", inet_ntoa(server.addr.sin_addr), ntohs(server.addr.sin_port));

    int ret = EXIT_SUCCESS;

    fd_set fdset;
    struct timeval timeout;
    while (server.running)
    {
        FD_ZERO(&fdset);
        FD_SET(server_socket_fd, &fdset);

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        int ret = select(server_socket_fd + 1, &fdset, NULL, NULL, &timeout);
        if (ret == -1)
            break;
        else if (ret > 0)
        {
            int client_socket = accept(server_socket_fd, NULL, NULL);
            if (client_socket == -1)
            {
                LOG_ERROR("accept: %s", strerror(errno));
                ret = EXIT_FAILURE;
                break;
            }

            LOG_DEBUG("new client connected: socket=%d", client_socket);

            // post request job for thread pool
            post_request_job((struct request_job){
                .client_socket = client_socket,
            });
        }
    }

    signal(SIGINT, old_handler);
    close(server_socket_fd);
    free_all_request_jobs();
    destroy_thread_pool(thread_pool);
    return ret;
}
