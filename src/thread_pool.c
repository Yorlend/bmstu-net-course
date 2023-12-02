#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "request.h"
#include "request_queue.h"
#include "thread_pool.h"

typedef void* (*thread_routine_t)(void*);

static void* worker_function(struct thread_pool *pool)
{
    LOG_DEBUG("worker thread %d started", pthread_self());

    struct request_job job;
    while (!pool->thread_stop_request)
    {
        if (get_request_job(&job) == EXIT_SUCCESS)
        {
            LOG_DEBUG("job received client_socket=%d thread_id=%d", job.client_socket, pthread_self());
            handle_request(job.client_socket);
        }
    }
    
    LOG_DEBUG("worker thread %d stopped", pthread_self());
    return NULL;
}

struct thread_pool* create_thread_pool(int num_threads)
{
    struct thread_pool* pool = calloc(1, sizeof(struct thread_pool));
    if (pool == NULL)
    {
        LOG_ERROR("malloc: %s", strerror(errno));
        return NULL;
    }

    pool->num_threads = num_threads;
    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_create(pool->threads + i, NULL, (thread_routine_t)worker_function, pool) != 0)
        {
            LOG_ERROR("pthread_create: %s", strerror(errno));
            pool->thread_stop_request = true;
            for (int j = 0; j < i; j++)
                pthread_join(pool->threads[j], NULL);
            free(pool);
            return NULL;
        }
    }

    LOG_DEBUG("thread pool created: num_threads=%d", num_threads);
    return pool;
}

void destroy_thread_pool(struct thread_pool* pool)
{
    pool->thread_stop_request = true;
    for (int i = 0; i < pool->num_threads; i++)
        pthread_join(pool->threads[i], NULL);
    free(pool);

    LOG_DEBUG("thread pool destroyed");
}
