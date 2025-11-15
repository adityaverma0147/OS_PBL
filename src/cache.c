#include "cache.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define CACHE_CAPACITY 8

typedef struct CacheEntry {
    char path[256];
    char *content;
    size_t size;
    time_t last_used;
} CacheEntry;

static CacheEntry cache[CACHE_CAPACITY];
static int cache_count = 0;
pthread_rwlock_t cache_lock;

void init_cache() {
    pthread_rwlock_init(&cache_lock, NULL);
}

char* get_from_cache(const char* path, size_t* size) {
    pthread_rwlock_rdlock(&cache_lock);

    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache[i].path, path) == 0) {
            cache[i].last_used = time(NULL);
            *size = cache[i].size;

            char* data = malloc(cache[i].size + 1);
            memcpy(data, cache[i].content, cache[i].size);
            data[cache[i].size] = '\0';

            pthread_rwlock_unlock(&cache_lock);
            printf("[CACHE HIT] %s (size: %zu bytes)\n", path, *size);
            return data;
        }
    }

    pthread_rwlock_unlock(&cache_lock);
    printf("[CACHE MISS] %s\n", path);
    return NULL;
}

static int find_lru_index() {
    int lru = 0;
    for (int i = 1; i < cache_count; i++) {
        if (cache[i].last_used < cache[lru].last_used)
            lru = i;
    }
    return lru;
}

void put_in_cache(const char* path, const char* data, size_t size) {
    pthread_rwlock_wrlock(&cache_lock);

    int index;
    if (cache_count < CACHE_CAPACITY) {
        index = cache_count++;
        printf("[CACHE ADD] %s (size: %zu bytes)\n", path, size);
    } else {
        index = find_lru_index();
        printf("[CACHE EVICT] %s --> REPLACED LRU\n", cache[index].path);
        printf("[CACHE ADD] %s (size: %zu bytes)\n", path, size);
        free(cache[index].content);
    }

    strcpy(cache[index].path, path);
    cache[index].content = malloc(size);
    memcpy(cache[index].content, data, size);
    cache[index].size = size;
    cache[index].last_used = time(NULL);

    pthread_rwlock_unlock(&cache_lock);
}
