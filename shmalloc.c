/*
* shmalloc - A lightweight and fast memory allocator designed for hobby operating systems.
 *
 * Written by: Kevin Alavik <kevin@alavik.se>
 * Licensed under the Apache License 2.0 (https://www.apache.org/licenses/LICENSE-2.0)
 *
 * Copyright (c) 2025
 */

#include <shmalloc.h>

/* --- shmalloc internal --- */
#define SHM_PAGE_SIZE 0x1000

/* --- shmalloc generic functions --- */
void *SHM_PREFIX(malloc)(size_t size) {
    (void)size;
    return NULL;
}

void *SHM_PREFIX(realloc)(void *ptr, size_t size) {
    (void)ptr;
    (void)size;
    return NULL;
}

void *SHM_PREFIX(calloc)(size_t num, size_t size) {
    (void)num;
    (void)size;
    return NULL;
}

void SHM_PREFIX(free)(void *ptr) {
    (void)ptr;
}
