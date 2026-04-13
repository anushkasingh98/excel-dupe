#include "sc_hashmap.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 64
#define LOAD_FACTOR_MAX 0.75

typedef struct {
    void *key;
    void *value;
    bool occupied;
    uint32_t psl;  /* probe sequence length for Robin Hood hashing */
} SCBucket;

struct SCHashMap {
    SCBucket *buckets;
    size_t capacity;
    size_t count;
    size_t key_size;
    size_t value_size;
    SCHashFunc hash_fn;
    SCEqualFunc equal_fn;
};

/* FNV-1a 64-bit hash */
uint64_t sc_hash_fnv1a(const void *key, size_t key_size) {
    const uint8_t *data = (const uint8_t *)key;
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < key_size; i++) {
        hash ^= data[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

bool sc_equal_memcmp(const void *a, const void *b, size_t key_size) {
    return memcmp(a, b, key_size) == 0;
}

SCHashMap *sc_hashmap_create(size_t key_size, size_t value_size,
                             SCHashFunc hash_fn, SCEqualFunc equal_fn) {
    SCHashMap *map = calloc(1, sizeof(SCHashMap));
    if (!map) return NULL;

    map->capacity = INITIAL_CAPACITY;
    map->count = 0;
    map->key_size = key_size;
    map->value_size = value_size;
    map->hash_fn = hash_fn ? hash_fn : sc_hash_fnv1a;
    map->equal_fn = equal_fn ? equal_fn : sc_equal_memcmp;
    map->buckets = calloc(map->capacity, sizeof(SCBucket));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    return map;
}

void sc_hashmap_destroy(SCHashMap *map, SCFreeFunc free_value) {
    if (!map) return;
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->buckets[i].occupied) {
            if (free_value && map->buckets[i].value) {
                void *val_ptr;
                memcpy(&val_ptr, map->buckets[i].value, sizeof(void *));
                free_value(val_ptr);
            }
            free(map->buckets[i].key);
            free(map->buckets[i].value);
        }
    }
    free(map->buckets);
    free(map);
}

static size_t bucket_index(const SCHashMap *map, const void *key) {
    uint64_t h = map->hash_fn(key, map->key_size);
    return (size_t)(h % map->capacity);
}

static bool hashmap_grow(SCHashMap *map);

bool sc_hashmap_set(SCHashMap *map, const void *key, const void *value) {
    if ((double)(map->count + 1) / (double)map->capacity > LOAD_FACTOR_MAX) {
        if (!hashmap_grow(map)) return false;
    }

    void *ins_key = malloc(map->key_size);
    void *ins_val = malloc(map->value_size);
    if (!ins_key || !ins_val) {
        free(ins_key);
        free(ins_val);
        return false;
    }
    memcpy(ins_key, key, map->key_size);
    memcpy(ins_val, value, map->value_size);

    uint32_t psl = 0;
    size_t idx = bucket_index(map, key);

    for (;;) {
        SCBucket *b = &map->buckets[idx];

        if (!b->occupied) {
            b->key = ins_key;
            b->value = ins_val;
            b->occupied = true;
            b->psl = psl;
            map->count++;
            return true;
        }

        /* Key already exists: update value */
        if (map->equal_fn(b->key, ins_key, map->key_size)) {
            free(ins_key);
            free(b->value);
            b->value = ins_val;
            return true;
        }

        /* Robin Hood: swap if current entry has lower PSL */
        if (b->psl < psl) {
            void *tmp_key = b->key;
            void *tmp_val = b->value;
            uint32_t tmp_psl = b->psl;

            b->key = ins_key;
            b->value = ins_val;
            b->psl = psl;

            ins_key = tmp_key;
            ins_val = tmp_val;
            psl = tmp_psl;
        }

        psl++;
        idx = (idx + 1) % map->capacity;
    }
}

void *sc_hashmap_get(const SCHashMap *map, const void *key) {
    size_t idx = bucket_index(map, key);
    uint32_t psl = 0;

    for (;;) {
        const SCBucket *b = &map->buckets[idx];
        if (!b->occupied || psl > b->psl) return NULL;
        if (map->equal_fn(b->key, key, map->key_size)) return b->value;
        psl++;
        idx = (idx + 1) % map->capacity;
    }
}

bool sc_hashmap_contains(const SCHashMap *map, const void *key) {
    return sc_hashmap_get(map, key) != NULL;
}

bool sc_hashmap_remove(SCHashMap *map, const void *key, SCFreeFunc free_value) {
    size_t idx = bucket_index(map, key);
    uint32_t psl = 0;

    for (;;) {
        SCBucket *b = &map->buckets[idx];
        if (!b->occupied || psl > b->psl) return false;

        if (map->equal_fn(b->key, key, map->key_size)) {
            if (free_value && b->value) {
                void *val_ptr;
                memcpy(&val_ptr, b->value, sizeof(void *));
                free_value(val_ptr);
            }
            free(b->key);
            free(b->value);
            b->occupied = false;
            b->psl = 0;
            map->count--;

            /* Backward shift deletion to maintain Robin Hood invariant */
            size_t next = (idx + 1) % map->capacity;
            while (map->buckets[next].occupied && map->buckets[next].psl > 0) {
                map->buckets[idx] = map->buckets[next];
                map->buckets[idx].psl--;
                map->buckets[next].occupied = false;
                map->buckets[next].key = NULL;
                map->buckets[next].value = NULL;
                map->buckets[next].psl = 0;
                idx = next;
                next = (next + 1) % map->capacity;
            }
            return true;
        }

        psl++;
        idx = (idx + 1) % map->capacity;
    }
}

size_t sc_hashmap_count(const SCHashMap *map) {
    return map ? map->count : 0;
}

void sc_hashmap_clear(SCHashMap *map, SCFreeFunc free_value) {
    if (!map) return;
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->buckets[i].occupied) {
            if (free_value && map->buckets[i].value) {
                void *val_ptr;
                memcpy(&val_ptr, map->buckets[i].value, sizeof(void *));
                free_value(val_ptr);
            }
            free(map->buckets[i].key);
            free(map->buckets[i].value);
            map->buckets[i].occupied = false;
            map->buckets[i].key = NULL;
            map->buckets[i].value = NULL;
            map->buckets[i].psl = 0;
        }
    }
    map->count = 0;
}

static bool hashmap_grow(SCHashMap *map) {
    size_t old_cap = map->capacity;
    SCBucket *old_buckets = map->buckets;

    map->capacity = old_cap * 2;
    map->buckets = calloc(map->capacity, sizeof(SCBucket));
    if (!map->buckets) {
        map->buckets = old_buckets;
        map->capacity = old_cap;
        return false;
    }
    map->count = 0;

    for (size_t i = 0; i < old_cap; i++) {
        if (old_buckets[i].occupied) {
            /* Re-insert: key and value are already allocated, transfer ownership */
            uint32_t psl = 0;
            size_t idx = bucket_index(map, old_buckets[i].key);

            void *ins_key = old_buckets[i].key;
            void *ins_val = old_buckets[i].value;

            for (;;) {
                SCBucket *b = &map->buckets[idx];
                if (!b->occupied) {
                    b->key = ins_key;
                    b->value = ins_val;
                    b->occupied = true;
                    b->psl = psl;
                    map->count++;
                    break;
                }
                if (b->psl < psl) {
                    void *tmp_key = b->key;
                    void *tmp_val = b->value;
                    uint32_t tmp_psl = b->psl;
                    b->key = ins_key;
                    b->value = ins_val;
                    b->psl = psl;
                    ins_key = tmp_key;
                    ins_val = tmp_val;
                    psl = tmp_psl;
                }
                psl++;
                idx = (idx + 1) % map->capacity;
            }
        }
    }
    free(old_buckets);
    return true;
}

/* Iterator */
void sc_hashmap_iter_init(SCHashMapIter *iter, const SCHashMap *map) {
    iter->map = map;
    iter->index = 0;
}

bool sc_hashmap_iter_next(SCHashMapIter *iter, void **key_out, void **value_out) {
    const SCHashMap *map = iter->map;
    while (iter->index < map->capacity) {
        const SCBucket *b = &map->buckets[iter->index];
        iter->index++;
        if (b->occupied) {
            if (key_out) *key_out = b->key;
            if (value_out) *value_out = b->value;
            return true;
        }
    }
    return false;
}
