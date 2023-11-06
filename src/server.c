#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "server.h"
#include "request_queue.h"
#include "thread_pool.h"

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
    printf("server stopped\n");
}

void init_server(const char* ip_address, int port)
{
    server.addr.sin_family = AF_INET;
    server.addr.sin_addr.s_addr = inet_addr(ip_address);
    server.addr.sin_port = htons(port);
    server.queue_capacity = 10;
    server.num_worker_threads = 4;
    server.running = false;
}

int run_server(void)
{
    // create socket and bind it to address
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    if (bind(server_socket_fd, (struct sockaddr*)&server.addr, sizeof(server.addr)) == -1) {
        perror("bind");
        return EXIT_FAILURE;
    }

    if (listen(server_socket_fd, server.queue_capacity) == -1) {
        perror("listen");
        return EXIT_FAILURE;
    }

    struct thread_pool* thread_pool = create_thread_pool(server.num_worker_threads);
    if (thread_pool == NULL) {
        perror("create_thread_pool");
        close(server_socket_fd);
        return EXIT_FAILURE;
    }

    server.running = true;
    __sighandler_t old_handler = signal(SIGINT, interrupt_handler);

    printf("server started at %s:%d\n", inet_ntoa(server.addr.sin_addr), ntohs(server.addr.sin_port));

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
                perror("accept");
                ret = EXIT_FAILURE;
                break;
            }

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
