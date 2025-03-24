/*
 * shmalloc - A lightweight and fast memory allocator designed for hobby operating systems.
 *
 * Written by: Kevin Alavik <kevin@alavik.se>
 * Licensed under the Apache License 2.0 (https://www.apache.org/licenses/LICENSE-2.0)
 *
 * Copyright (c) 2025
 */
#include <shmalloc.h>
#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

/* --- shmalloc wrapper functions --- */
static pthread_mutex_t shmalloc_mutex = PTHREAD_MUTEX_INITIALIZER;

int shmalloc_lock() {
    if (pthread_mutex_lock(&shmalloc_mutex) != 0) {
        return -1;
    }
    return 0;
}

int shmalloc_unlock() {
    if (pthread_mutex_unlock(&shmalloc_mutex) != 0) {
        return -1;
    }
    return 0;
}

void *shmalloc_alloc_pages(size_t size) {
    size_t page_size = getpagesize();
    size_t aligned_size = (size + page_size - 1) & ~(page_size - 1);
    void *ptr = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    return ptr;
}

void *shmalloc_free_pages(void *ptr, size_t size) {
    size_t page_size = getpagesize();
    size_t aligned_size = (size + page_size - 1) & ~(page_size - 1);
    if (munmap(ptr, aligned_size) != 0) {
        return NULL;
    }
    return ptr;
}

void shmalloc_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/* --- test entry --- */
int main() {
    void* test = sh_malloc(123);
    assert(test);
    return 0;
}