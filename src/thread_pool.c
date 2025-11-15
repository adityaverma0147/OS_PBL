#include "thread_pool.h"
#include "client_handler.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void thread_pool_init(ThreadPool* pool, int thread_count, int queue_capacity) {
    pool->queue = malloc(sizeof(int) * queue_capacity);
    pool->capacity = queue_capacity;
    pool->front = 0;
    pool->rear = 0;
    pool->count = 0;
    pool->thread_count = thread_count;
    pool->stop = 0;

    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pthread_cond_init(&pool->not_full, NULL);

    pool->threads = malloc(sizeof(pthread_t) * thread_count);
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&pool->threads[i], NULL, thread_worker, pool);
    }
}

void thread_pool_destroy(ThreadPool* pool) {
    pthread_mutex_lock(&pool->lock);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    free(pool->queue);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->not_full);
}

void thread_pool_add(ThreadPool* pool, int client_socket) {
    pthread_mutex_lock(&pool->lock);

    while (pool->count == pool->capacity) {
        pthread_cond_wait(&pool->not_full, &pool->lock);
    }

    pool->queue[pool->rear] = client_socket;
    pool->rear = (pool->rear + 1) % pool->capacity;
    pool->count++;

    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);
}

void* thread_worker(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;

    while (1) {
        pthread_mutex_lock(&pool->lock);
        while (pool->count == 0 && !pool->stop) {
            pthread_cond_wait(&pool->not_empty, &pool->lock);
        }

        if (pool->stop) {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }

        int client_socket = pool->queue[pool->front];
        pool->front = (pool->front + 1) % pool->capacity;
        pool->count--;

        pthread_cond_signal(&pool->not_full);
        pthread_mutex_unlock(&pool->lock);


        int *pclient = malloc(sizeof(int));
if (!pclient) {
    // allocation failure — close socket to avoid leak and continue
    perror("malloc");
    close(client_socket);
} else {
    *pclient = client_socket;
    // handle_client() expects a malloc'd pointer and will free it itself
    handle_client(pclient);
}

        // // Process the client
        // handle_client(&client_socket);
    }

    return NULL;
}
