#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

typedef struct {
    int* queue;
    int capacity;
    int front;
    int rear;
    int count;

    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;

    pthread_t* threads;
    int thread_count;
    int stop;
} ThreadPool;

void thread_pool_init(ThreadPool* pool, int thread_count, int queue_capacity);
void thread_pool_destroy(ThreadPool* pool);
void thread_pool_add(ThreadPool* pool, int client_socket);
void* thread_worker(void* arg);

#endif
