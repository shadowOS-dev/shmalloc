/*
 * shmalloc - A lightweight and fast memory allocator designed for hobby operating systems.
 *
 * Written by: Kevin Alavik <kevin@alavik.se>
 * Licensed under the Apache License 2.0 (https://www.apache.org/licenses/LICENSE-2.0)
 *
 * Copyright (c) 2025
 */

#ifndef _SHMALLOC_H_
#define _SHMALLOC_H_

/* --- shmalloc defines --- */
#define SHM_PREFIX(func) sh_##func

/* --- shmalloc types --- */
#ifndef size_t
#if defined(__x86_64__) || defined(__aarch64__) || defined(__ppc64__) || defined(__sparc64__) ||                       \
        (defined(__riscv) && (__riscv_xlen == 64))
typedef unsigned long size_t;
#elif defined(__i386__) || defined(__arm__) || defined(__mips__) || (defined(__riscv) && (__riscv_xlen == 32))
typedef unsigned int size_t;
#else
#error "Unsupported architecture: Please define 'size_t' for your architecture or update the configuration."
#endif
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/* --- shmalloc function wrappers --- */
extern int shmalloc_lock();
extern int shmalloc_unlock();
extern void *shmalloc_alloc_pages(size_t);
extern void *shmalloc_free_pages(void *, size_t);
extern void shmalloc_printf(const char *, ...); // If this isn't defined, logging is disabled.

/* --- shmalloc function declarations --- */
extern void *SHM_PREFIX(malloc)(size_t);
extern void *SHM_PREFIX(realloc)(void *, size_t);
extern void *SHM_PREFIX(calloc)(size_t, size_t);
extern void SHM_PREFIX(free)(void *);

#endif // _SHMALLOC_H_
