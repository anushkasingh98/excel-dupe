#ifndef SC_HASHMAP_H
#define SC_HASHMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct SCHashMap SCHashMap;

/* Callback types */
typedef uint64_t (*SCHashFunc)(const void *key, size_t key_size);
typedef bool (*SCEqualFunc)(const void *a, const void *b, size_t key_size);
typedef void (*SCFreeFunc)(void *ptr);

/* Create/destroy */
SCHashMap *sc_hashmap_create(size_t key_size, size_t value_size,
                             SCHashFunc hash_fn, SCEqualFunc equal_fn);
void sc_hashmap_destroy(SCHashMap *map, SCFreeFunc free_value);

/* Operations */
bool sc_hashmap_set(SCHashMap *map, const void *key, const void *value);
void *sc_hashmap_get(const SCHashMap *map, const void *key);
bool sc_hashmap_remove(SCHashMap *map, const void *key, SCFreeFunc free_value);
bool sc_hashmap_contains(const SCHashMap *map, const void *key);
size_t sc_hashmap_count(const SCHashMap *map);
void sc_hashmap_clear(SCHashMap *map, SCFreeFunc free_value);

/* Iteration */
typedef struct {
    const SCHashMap *map;
    size_t index;
} SCHashMapIter;

void sc_hashmap_iter_init(SCHashMapIter *iter, const SCHashMap *map);
bool sc_hashmap_iter_next(SCHashMapIter *iter, void **key_out, void **value_out);

/* Default hash: FNV-1a */
uint64_t sc_hash_fnv1a(const void *key, size_t key_size);
bool sc_equal_memcmp(const void *a, const void *b, size_t key_size);

#endif /* SC_HASHMAP_H */
