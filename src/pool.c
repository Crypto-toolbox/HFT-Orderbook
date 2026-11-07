/**
 * Fixed-size object pool.
 *
 * Objects are carved out of slabs (64, 128, ... up to 65536 objects each)
 * and recycled through a LIFO free list, so steady-state allocation is a
 * pointer pop and the most recently freed (cache-warm) object is reused
 * first. destroyPool() releases everything at once.
 *
 * Under AddressSanitizer, free objects are poisoned so use-after-free on
 * pooled objects is still reported.
 */
#include <stdlib.h>
#include "hftlob.h"

#if defined(__SANITIZE_ADDRESS__)
#  define HFTLOB_ASAN 1
#elif defined(__has_feature)
#  if __has_feature(address_sanitizer)
#    define HFTLOB_ASAN 1
#  endif
#endif

#ifdef HFTLOB_ASAN
#  include <sanitizer/asan_interface.h>
#  define POISON(p, n)   ASAN_POISON_MEMORY_REGION((p), (n))
#  define UNPOISON(p, n) ASAN_UNPOISON_MEMORY_REGION((p), (n))
#else
#  define POISON(p, n)   ((void)(p), (void)(n))
#  define UNPOISON(p, n) ((void)(p), (void)(n))
#endif

#define POOL_ALIGN 16
#define POOL_FIRST_SLAB 64
#define POOL_MAX_SLAB 65536

struct PoolSlab{
    struct PoolSlab *next;
    size_t bytes;
};

/* Slab header size, rounded up so objects stay POOL_ALIGN-aligned. */
#define SLAB_HEADER (((sizeof(struct PoolSlab) + POOL_ALIGN - 1) / POOL_ALIGN) * POOL_ALIGN)

void
initPool(Pool *pool, size_t objSize){
    if(objSize < sizeof(void *)){
        objSize = sizeof(void *);
    }
    pool->objSize = (objSize + POOL_ALIGN - 1) / POOL_ALIGN * POOL_ALIGN;
    pool->nextSlabObjects = POOL_FIRST_SLAB;
    pool->freeList = NULL;
    pool->slabs = NULL;
}

static int
addSlab(Pool *pool){
    size_t count = pool->nextSlabObjects;
    size_t bytes = SLAB_HEADER + count * pool->objSize;
    struct PoolSlab *slab = malloc(bytes);
    char *obj;
    size_t i;
    if(slab == NULL){
        return 0;
    }
    slab->next = pool->slabs;
    slab->bytes = bytes;
    pool->slabs = slab;
    if(pool->nextSlabObjects < POOL_MAX_SLAB){
        pool->nextSlabObjects *= 2;
    }

    /* Thread the new objects onto the free list, lowest address first. */
    obj = (char *)slab + SLAB_HEADER + (count - 1) * pool->objSize;
    for(i = 0; i < count; ++i, obj -= pool->objSize){
        *(void **)obj = pool->freeList;
        pool->freeList = obj;
        POISON(obj, pool->objSize);
    }
    return 1;
}

void*
poolAlloc(Pool *pool){
    /**
     * Return an uninitialised object, or NULL if out of memory.
     */
    void *obj;
    if(pool->freeList == NULL && !addSlab(pool)){
        return NULL;
    }
    obj = pool->freeList;
    UNPOISON(obj, pool->objSize);
    pool->freeList = *(void **)obj;
    return obj;
}

void
poolFree(Pool *pool, void *obj){
    /**
     * Return an object from this pool to it. obj may be NULL.
     */
    if(obj == NULL){
        return;
    }
    *(void **)obj = pool->freeList;
    pool->freeList = obj;
    POISON(obj, pool->objSize);
}

void
destroyPool(Pool *pool){
    /**
     * Release all slabs; every object from this pool becomes invalid.
     */
    struct PoolSlab *slab = pool->slabs;
    while(slab != NULL){
        struct PoolSlab *next;
        UNPOISON(slab, slab->bytes);
        next = slab->next;
        free(slab);
        slab = next;
    }
    pool->slabs = NULL;
    pool->freeList = NULL;
}
