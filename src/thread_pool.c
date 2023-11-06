#include <stdio.h>
#include <stdlib.h>

#include "request.h"
#include "request_queue.h"
#include "thread_pool.h"

typedef void* (*thread_routine_t)(void*);

static void* worker_function(struct thread_pool *pool)
{
    struct request_job job;
    while (!pool->thread_stop_request)
        if (get_request_job(&job) == EXIT_SUCCESS)
            handle_request(job.client_socket);
    return NULL;
}

struct thread_pool* create_thread_pool(int num_threads)
{
    struct thread_pool* pool = calloc(1, sizeof(struct thread_pool));
    if (pool == NULL)
    {
        perror("malloc");
        return NULL;
    }

    pool->num_threads = num_threads;
    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_create(pool->threads + i, NULL, (thread_routine_t)worker_function, pool) != 0)
        {
            perror("pthread_create");
            pool->thread_stop_request = true;
            for (int j = 0; j < i; j++)
                pthread_join(pool->threads[j], NULL);
            free(pool);
            return NULL;
        }
    }

    return pool;
}

void destroy_thread_pool(struct thread_pool* pool)
{
    pool->thread_stop_request = true;
    for (int i = 0; i < pool->num_threads; i++)
        pthread_join(pool->threads[i], NULL);
    free(pool);
}
