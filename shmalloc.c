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
#define SHM_LOG(fmt, ...) shmalloc_printf("[shmalloc] " fmt "\n", ##__VA_ARGS__)
#define SHM_UNIMPLEMENTED() SHM_LOG("%s is unimplemented", __FUNCTION__)
#define SHM_DEBUG(fmt, ...) shmalloc_printf("[shmalloc DEBUG] " fmt "\n", ##__VA_ARGS__)

/* --- shmalloc slab --- */
#define SLAB_MAX_CLASSES 16

typedef struct slab {
    struct slab *next;
    void *freelist;
    size_t freecount;
    size_t objsize;
} slab_t;

typedef struct cache {
    slab_t *partial;
    slab_t *full;
    slab_t *empty;
    size_t objsize;
} slab_cache_t;

static slab_cache_t slab_caches[SLAB_MAX_CLASSES];
static const size_t SLAB_SIZES[SLAB_MAX_CLASSES] = {16,   32,   64,    128,   256,   512,    1024,   2048,
                                                    4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288};
static int slab_initialized = 0;

static slab_cache_t *get_slab_cache(size_t size) {
    for (size_t i = 0; i < SLAB_MAX_CLASSES; i++) {
        if (size <= SLAB_SIZES[i]) {
            return &slab_caches[i];
        }
    }
    return NULL;
}

static slab_t *alloc_slab(size_t objsize) {
    slab_t *slab = (slab_t *) shmalloc_alloc_pages(1);
    if (!slab) {
        SHM_LOG("Failed to allocate slab of size %zu", objsize);
        return NULL;
    }

    slab->objsize = objsize;
    slab->freecount = (PAGE_SIZE - sizeof(slab_t)) / objsize;
    slab->next = NULL;
    slab->freelist = (void *) ((uintptr_t) slab + sizeof(slab_t));

    void *current = slab->freelist;
    for (size_t i = 0; i < slab->freecount - 1; i++) {
        *(void **) current = (void *) ((uintptr_t) current + objsize);
        current = *(void **) current;
    }
    *(void **) current = NULL;
    return slab;
}

void init_slab(void) {
    for (size_t i = 0; i < SLAB_MAX_CLASSES; i++) {
        slab_caches[i].objsize = SLAB_SIZES[i];
        slab_caches[i].partial = NULL;
        slab_caches[i].full = NULL;
        slab_caches[i].empty = NULL;
    }
}

static void ensure_slab_initialized(void) {
    if (!slab_initialized) {
        shmalloc_lock();
        if (!slab_initialized) {
            init_slab();
            slab_initialized = 1;
        }
        shmalloc_unlock();
    }
}

/* --- shmalloc generic functions --- */
void *SHM_PREFIX(malloc)(size_t size) {
    ensure_slab_initialized();
    shmalloc_lock();

    if (size > PAGE_SIZE) {
        SHM_DEBUG("Large allocation requested: %zu bytes, allocating across multiple slabs", size);
        size_t remaining_size = size;
        void *allocated_memory = NULL;
        void *current_ptr = NULL;

        while (remaining_size > 0) {
            slab_t *slab = alloc_slab(SLAB_SIZES[0]);
            if (!slab) {
                SHM_LOG("Failed to allocate slab during multi-slab allocation");
                shmalloc_unlock();
                return NULL;
            }

            if (!allocated_memory) {
                allocated_memory = slab->freelist;
            }

            current_ptr = slab->freelist;
            remaining_size -= PAGE_SIZE - sizeof(slab_t);
        }

        shmalloc_unlock();
        return allocated_memory;
    }

    slab_cache_t *cache = get_slab_cache(size);
    if (!cache) {
        SHM_LOG("No suitable slab cache for size %zu", size);
        shmalloc_unlock();
        return NULL;
    }

    if (!cache->partial) {
        cache->partial = alloc_slab(cache->objsize);
        if (!cache->partial) {
            shmalloc_unlock();
            return NULL;
        }
    }

    slab_t *slab = cache->partial;
    void *obj = slab->freelist;
    slab->freelist = *(void **) obj;
    slab->freecount--;

    if (slab->freecount == 0) {
        cache->partial = slab->next;
        slab->next = cache->full;
        cache->full = slab;
    }

    shmalloc_unlock();
    return obj;
}

void SHM_PREFIX(free)(void *ptr) {
    if (!ptr) {
        SHM_LOG("Attempt to free NULL pointer");
        return;
    }

    ensure_slab_initialized();
    shmalloc_lock();

    slab_t *slab = (slab_t *) ((uintptr_t) ptr & ~(PAGE_SIZE - 1));
    if (slab->freelist != ptr && slab->freecount == 0) {
        SHM_LOG("Memory corruption detected when freeing pointer %p", ptr);
        shmalloc_unlock();
        return;
    }

    *(void **) ptr = slab->freelist;
    slab->freelist = ptr;
    slab->freecount++;

    slab_cache_t *cache = get_slab_cache(slab->objsize);
    if (!cache) {
        SHM_LOG("Slab cache for size %zu not found", slab->objsize);
        shmalloc_unlock();
        return;
    }

    if (slab->freecount == 1) {
        slab->next = cache->partial;
        cache->partial = slab;
    } else if (slab->freecount == (PAGE_SIZE - sizeof(slab_t)) / slab->objsize) {
        slab->next = cache->empty;
        cache->empty = slab;
    }

    shmalloc_unlock();
}

void *SHM_PREFIX(realloc)(void *ptr, size_t size) {
    ensure_slab_initialized();
    if (!ptr)
        return SHM_PREFIX(malloc)(size);

    shmalloc_lock();
    slab_t *slab = (slab_t *) ((uintptr_t) ptr & ~(PAGE_SIZE - 1));
    if (slab->objsize >= size) {
        SHM_DEBUG("Reallocation not needed, existing object size %zu is sufficient", slab->objsize);
        shmalloc_unlock();
        return ptr;
    }
    shmalloc_unlock();

    void *newptr = SHM_PREFIX(malloc)(size);
    if (newptr) {
        shmalloc_memcpy(newptr, ptr, slab->objsize);
        SHM_PREFIX(free)(ptr);
    }

    return newptr;
}

void *SHM_PREFIX(calloc)(size_t num, size_t size) {
    ensure_slab_initialized();
    size_t total_size = num * size;
    void *ptr = SHM_PREFIX(malloc)(total_size);
    if (ptr) {
        shmalloc_memset(ptr, 0, total_size);
    }
    return ptr;
}
