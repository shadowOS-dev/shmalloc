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
#define SHM_LOG(fmt, ...)                                                                                              \
    shmalloc_printf("shmalloc: ");                                                                                     \
    shmalloc_printf(fmt, ##__VA_ARGS__);                                                                               \
    shmalloc_printf("\n")

/* --- shmalloc generic functions --- */
void *SHM_PREFIX(malloc)(size_t size) {
    (void) size;
    SHM_LOG("malloc is unimplemented");
    return NULL;
}

void *SHM_PREFIX(realloc)(void *ptr, size_t size) {
    (void) ptr;
    (void) size;
    SHM_LOG("realloc is unimplemented");
    return NULL;
}

void *SHM_PREFIX(calloc)(size_t num, size_t size) {
    (void) num;
    (void) size;
    SHM_LOG("calloc is unimplemented");
    return NULL;
}

void SHM_PREFIX(free)(void *ptr) {
    SHM_LOG("free is unimplemented");
    (void) ptr;
}
