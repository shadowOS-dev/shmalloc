/*
 * shmalloc - A lightweight and fast memory allocator designed for hobby operating systems.
 *
 * Written by: Kevin Alavik <kevin@alavik.se>
 * Licensed under the Apache License 2.0 (https://www.apache.org/licenses/LICENSE-2.0)
 *
 * Copyright (c) 2025
 */
#include <pthread.h>
#include <shmalloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/* --- Global Variables --- */
static size_t total_allocs = 10000000;

#ifdef __APPLE__
static pthread_mutex_t shmalloc_lock_var = PTHREAD_MUTEX_INITIALIZER;
#else
static pthread_spinlock_t shmalloc_lock_var;
#endif

/* --- shmalloc wrapper functions --- */
int shmalloc_lock() {
#ifdef __APPLE__
    return pthread_mutex_lock(&shmalloc_lock_var);
#else
    return pthread_spin_lock(&shmalloc_lock_var);
#endif
}

int shmalloc_unlock() {
#ifdef __APPLE__
    return pthread_mutex_unlock(&shmalloc_lock_var);
#else
    return pthread_spin_unlock(&shmalloc_lock_var);
#endif
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

void *shmalloc_memcpy(void *dest, const void *src, size_t n) { return memcpy(dest, src, n); }
void *shmalloc_memset(void *s, int c, size_t n) { return memset(s, c, n); }

/* --- Benchmark Functions --- */
void benchmark() {
    size_t malloc_size = 128;
    size_t allocations = total_allocs;
    void *ptr = NULL;

    clock_t start = clock();

    for (size_t i = 0; i < allocations; ++i) {
        ptr = sh_malloc(malloc_size);
        if (!ptr) {
            fprintf(stderr, "Allocation failed at iteration %zu\n", i);
            return;
        }
        sh_free(ptr);
    }

    clock_t end = clock();
    double time_taken = (double) (end - start) / CLOCKS_PER_SEC;
    shmalloc_printf("Completed %zu allocations in %f seconds\n", total_allocs, time_taken);
}

/* --- Test Entry --- */
int main() {
#ifndef __APPLE__
    pthread_spin_init(&shmalloc_lock_var, 0);
#endif
    benchmark();
#ifndef __APPLE__
    pthread_spin_destroy(&shmalloc_lock_var);
#endif
    return 0;
}
