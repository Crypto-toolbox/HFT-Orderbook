/**
 * Open-addressing hash map with uint64_t keys and pointer values.
 *
 * Slots hold {key, value} inline in one flat array, so a lookup is
 * usually a single cache line: no per-entry allocation, no key copies.
 * Linear probing, load factor <= 1/2, and backward-shift deletion (no
 * tombstones). A NULL value marks an empty slot, so values must not be
 * NULL.
 *
 * Used by the Book to map order ids to Orders and prices to Limits.
 */
#include <stdint.h>
#include <stdlib.h>
#include "hftlob.h"

#define MAP_MIN_CAPACITY 16

static uint64_t
mixKey(uint64_t key){
    /* splitmix64 finaliser: spreads sequential ids across the table. */
    key ^= key >> 30;
    key *= 0xbf58476d1ce4e5b9ULL;
    key ^= key >> 27;
    key *= 0x94d049bb133111ebULL;
    key ^= key >> 31;
    return key;
}

int
initMap(HashMap *map, size_t capacity){
    size_t cap = MAP_MIN_CAPACITY;
    while(cap < capacity){
        cap <<= 1;
    }
    map->slots = calloc(cap, sizeof(MapSlot));
    map->capacity = map->slots ? cap : 0;
    map->size = 0;
    return map->slots != NULL;
}

void
freeMap(HashMap *map){
    free(map->slots);
    map->slots = NULL;
    map->capacity = 0;
    map->size = 0;
}

static size_t
findIndex(const HashMap *map, uint64_t key){
    /* Index of key's slot, or of the empty slot where it would go. */
    size_t mask = map->capacity - 1;
    size_t i = mixKey(key) & mask;
    while(map->slots[i].value != NULL && map->slots[i].key != key){
        i = (i + 1) & mask;
    }
    return i;
}

static int
growMap(HashMap *map){
    HashMap bigger;
    size_t i;
    if(!initMap(&bigger, map->capacity * 2)){
        return 0;
    }
    for(i = 0; i < map->capacity; ++i){
        if(map->slots[i].value != NULL){
            bigger.slots[findIndex(&bigger, map->slots[i].key)] = map->slots[i];
        }
    }
    bigger.size = map->size;
    free(map->slots);
    *map = bigger;
    return 1;
}

void*
mapGet(const HashMap *map, uint64_t key){
    return map->slots[findIndex(map, key)].value;
}

int
mapPut(HashMap *map, uint64_t key, void *value){
    /**
     * Insert or overwrite; value must not be NULL.
     * Returns 1 on success, 0 if out of memory.
     */
    size_t i;
    if(value == NULL){
        return 0;
    }
    if((map->size + 1) * 2 > map->capacity && !growMap(map)){
        return 0;
    }
    i = findIndex(map, key);
    if(map->slots[i].value == NULL){
        map->size++;
    }
    map->slots[i].key = key;
    map->slots[i].value = value;
    return 1;
}

void*
mapRemove(HashMap *map, uint64_t key){
    /**
     * Remove a key and return its value (NULL if absent).
     */
    size_t mask = map->capacity - 1;
    size_t i = findIndex(map, key);
    size_t j = i;
    void *value = map->slots[i].value;
    if(value == NULL){
        return NULL;
    }
    map->slots[i].value = NULL;
    map->size--;

    /* Backward-shift: pull later entries of the probe run into the gap. */
    for(;;){
        size_t home;
        j = (j + 1) & mask;
        if(map->slots[j].value == NULL){
            break;
        }
        home = mixKey(map->slots[j].key) & mask;
        /* Entry at j may stay if its home lies cyclically in (i, j]. */
        if(i <= j ? (i < home && home <= j) : (i < home || home <= j)){
            continue;
        }
        map->slots[i] = map->slots[j];
        map->slots[j].value = NULL;
        i = j;
    }
    return value;
}
