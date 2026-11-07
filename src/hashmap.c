/**
 * A small separate-chaining hash map with byte-string keys.
 *
 * Keys are copied into the map, values are borrowed pointers. Used by the
 * Book to map order ids to Orders and prices to Limits in O(1).
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "hftlob.h"

#define MAP_MIN_CAPACITY 16

static uint64_t
hashBytes(const void *key, size_t keyLen){
    /* FNV-1a */
    const unsigned char *bytes = key;
    uint64_t hash = 1469598103934665603ULL;
    size_t i;
    for(i = 0; i < keyLen; ++i){
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

int
initMap(HashMap *map, size_t capacity){
    if(capacity < MAP_MIN_CAPACITY){
        capacity = MAP_MIN_CAPACITY;
    }
    map->buckets = calloc(capacity, sizeof(MapEntry *));
    map->capacity = map->buckets ? capacity : 0;
    map->size = 0;
    return map->buckets != NULL;
}

void
freeMap(HashMap *map){
    size_t i;
    for(i = 0; i < map->capacity; ++i){
        MapEntry *entry = map->buckets[i];
        while(entry != NULL){
            MapEntry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = NULL;
    map->capacity = 0;
    map->size = 0;
}

static MapEntry **
findSlot(const HashMap *map, const void *key, size_t keyLen, uint64_t hash){
    MapEntry **slot = &map->buckets[hash % map->capacity];
    while(*slot != NULL){
        if((*slot)->hash == hash && (*slot)->keyLen == keyLen
                && memcmp((*slot)->key, key, keyLen) == 0){
            break;
        }
        slot = &(*slot)->next;
    }
    return slot;
}

static int
growMap(HashMap *map){
    size_t newCapacity = map->capacity * 2;
    MapEntry **newBuckets = calloc(newCapacity, sizeof(MapEntry *));
    size_t i;
    if(newBuckets == NULL){
        return 0;
    }
    for(i = 0; i < map->capacity; ++i){
        MapEntry *entry = map->buckets[i];
        while(entry != NULL){
            MapEntry *next = entry->next;
            size_t idx = entry->hash % newCapacity;
            entry->next = newBuckets[idx];
            newBuckets[idx] = entry;
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = newBuckets;
    map->capacity = newCapacity;
    return 1;
}

void*
mapGet(const HashMap *map, const void *key, size_t keyLen){
    MapEntry *entry = *findSlot(map, key, keyLen, hashBytes(key, keyLen));
    return entry ? entry->value : NULL;
}

int
mapPut(HashMap *map, const void *key, size_t keyLen, void *value){
    /**
     * Insert or overwrite. Returns 1 on success, 0 if out of memory.
     */
    uint64_t hash = hashBytes(key, keyLen);
    MapEntry **slot = findSlot(map, key, keyLen, hash);
    MapEntry *entry;

    if(*slot != NULL){
        (*slot)->value = value;
        return 1;
    }
    if((map->size + 1) * 4 > map->capacity * 3){
        if(!growMap(map)){
            return 0;
        }
        slot = findSlot(map, key, keyLen, hash);
    }

    entry = malloc(sizeof(MapEntry));
    if(entry == NULL){
        return 0;
    }
    entry->key = malloc(keyLen ? keyLen : 1);
    if(entry->key == NULL){
        free(entry);
        return 0;
    }
    memcpy(entry->key, key, keyLen);
    entry->keyLen = keyLen;
    entry->hash = hash;
    entry->value = value;
    entry->next = NULL;
    *slot = entry;
    map->size++;
    return 1;
}

void*
mapRemove(HashMap *map, const void *key, size_t keyLen){
    /**
     * Remove a key and return its value (NULL if absent).
     */
    MapEntry **slot = findSlot(map, key, keyLen, hashBytes(key, keyLen));
    MapEntry *entry = *slot;
    void *value;
    if(entry == NULL){
        return NULL;
    }
    value = entry->value;
    *slot = entry->next;
    free(entry->key);
    free(entry);
    map->size--;
    return value;
}
