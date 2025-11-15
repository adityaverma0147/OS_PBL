#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>

void init_cache();
char* get_from_cache(const char* path, size_t* size);
void put_in_cache(const char* path, const char* data, size_t size);

#endif
