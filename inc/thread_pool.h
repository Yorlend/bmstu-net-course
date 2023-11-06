#pragma once

#include <stdbool.h>
#include <threads.h>
#include <pthread.h>

#define MAX_POOL_THREADS 32

struct thread_pool
{
    pthread_t threads[MAX_POOL_THREADS];
    int num_threads;
    bool thread_stop_request;
};

struct thread_pool* create_thread_pool(int num_threads);
void destroy_thread_pool(struct thread_pool *pool);
