#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "request_queue.h"

struct job_list_node
{
    struct request_job job;
    struct job_list_node* next;
};

static bool queue_destroyed = false;
static struct job_list_node* queue = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

inline static void lock(void)
{
    if (pthread_mutex_lock(&mutex) != 0)
    {
        LOG_ERROR("pthread_mutex_lock: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

inline static void unlock(void)
{
    if (pthread_mutex_unlock(&mutex) != 0)
    {
        LOG_ERROR("pthread_mutex_unlock: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

inline static void wakeup(void)
{
    if (pthread_cond_signal(&cond) != 0)
    {
        LOG_ERROR("pthread_cond_signal: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

inline static void wakeup_all(void)
{
    if (pthread_cond_broadcast(&cond) != 0)
    {
        LOG_ERROR("pthread_cond_broadcast: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

inline static void wait(void)
{
    if (pthread_cond_wait(&cond, &mutex) != 0)
    {
        LOG_ERROR("pthread_cond_wait: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int post_request_job(struct request_job job)
{
    struct job_list_node* node = calloc(1, sizeof(struct job_list_node));
    if (node == NULL)
    {
        LOG_ERROR("calloc: %s", strerror(errno));
        return EXIT_FAILURE;
    }
    node->job = job;

    // append job
    lock();
    struct job_list_node** last = &queue;
    while (*last != NULL)
        last = &(*last)->next;
    *last = node;
    wakeup();
    unlock();

    LOG_DEBUG("job posted client_socket=%d", job.client_socket);
    return EXIT_SUCCESS;
}

void free_all_request_jobs(void)
{
    lock();
    while (queue != NULL)
    {
        struct job_list_node* next = queue->next;
        free(queue);
        queue = next;
    }
    queue_destroyed = true;
    wakeup_all();
    unlock();

    LOG_DEBUG("request jobs queue destroyed");
}

int get_request_job(struct request_job* job)
{
    int ret = EXIT_SUCCESS;

    lock();
    while (queue == NULL && !queue_destroyed)
        wait();
    if (queue == NULL)
        ret = EXIT_FAILURE;
    else
    {
        *job = queue->job;
        struct job_list_node* next = queue->next;
        free(queue);
        queue = next;
    }
    unlock();
    return ret;
}
