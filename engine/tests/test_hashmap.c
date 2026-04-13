#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sc_hashmap.h"

static void test_create_destroy(void) {
    SCHashMap *map = sc_hashmap_create(sizeof(int), sizeof(int), NULL, NULL);
    assert(map != NULL);
    assert(sc_hashmap_count(map) == 0);
    sc_hashmap_destroy(map, NULL);
    printf("  PASS: create/destroy\n");
}

static void test_set_get(void) {
    SCHashMap *map = sc_hashmap_create(sizeof(int), sizeof(int), NULL, NULL);

    int key = 42, value = 100;
    assert(sc_hashmap_set(map, &key, &value));
    assert(sc_hashmap_count(map) == 1);

    void *result = sc_hashmap_get(map, &key);
    assert(result != NULL);
    int got;
    memcpy(&got, result, sizeof(int));
    assert(got == 100);

    /* Key not found */
    int missing = 99;
    assert(sc_hashmap_get(map, &missing) == NULL);

    sc_hashmap_destroy(map, NULL);
    printf("  PASS: set/get\n");
}

static void test_overwrite(void) {
    SCHashMap *map = sc_hashmap_create(sizeof(int), sizeof(int), NULL, NULL);

    int key = 1, v1 = 10, v2 = 20;
    sc_hashmap_set(map, &key, &v1);
    sc_hashmap_set(map, &key, &v2);
    assert(sc_hashmap_count(map) == 1);

    void *result = sc_hashmap_get(map, &key);
    int got;
    memcpy(&got, result, sizeof(int));
    assert(got == 20);

    sc_hashmap_destroy(map, NULL);
    printf("  PASS: overwrite\n");
}

static void test_remove(void) {
    SCHashMap *map = sc_hashmap_create(sizeof(int), sizeof(int), NULL, NULL);

    int key = 5, value = 50;
    sc_hashmap_set(map, &key, &value);
    assert(sc_hashmap_count(map) == 1);

    assert(sc_hashmap_remove(map, &key, NULL));
    assert(sc_hashmap_count(map) == 0);
    assert(sc_hashmap_get(map, &key) == NULL);

    /* Remove non-existent */
    assert(!sc_hashmap_remove(map, &key, NULL));

    sc_hashmap_destroy(map, NULL);
    printf("  PASS: remove\n");
}

static void test_many_entries(void) {
    SCHashMap *map = sc_hashmap_create(sizeof(int), sizeof(int), NULL, NULL);

    /* Insert 1000 entries to test growth */
    for (int i = 0; i < 1000; i++) {
        int val = i * 10;
        sc_hashmap_set(map, &i, &val);
    }
    assert(sc_hashmap_count(map) == 1000);

    /* Verify all entries */
    for (int i = 0; i < 1000; i++) {
        void *result = sc_hashmap_get(map, &i);
        assert(result != NULL);
        int got;
        memcpy(&got, result, sizeof(int));
        assert(got == i * 10);
    }

    /* Remove every other entry */
    for (int i = 0; i < 1000; i += 2) {
        assert(sc_hashmap_remove(map, &i, NULL));
    }
    assert(sc_hashmap_count(map) == 500);

    /* Verify remaining */
    for (int i = 1; i < 1000; i += 2) {
        void *result = sc_hashmap_get(map, &i);
        assert(result != NULL);
        int got;
        memcpy(&got, result, sizeof(int));
        assert(got == i * 10);
    }

    sc_hashmap_destroy(map, NULL);
    printf("  PASS: many entries (1000 insert, 500 remove)\n");
}

static void test_iterator(void) {
    SCHashMap *map = sc_hashmap_create(sizeof(int), sizeof(int), NULL, NULL);

    for (int i = 0; i < 10; i++) {
        int val = i * 100;
        sc_hashmap_set(map, &i, &val);
    }

    SCHashMapIter iter;
    sc_hashmap_iter_init(&iter, map);
    void *key, *val;
    int count = 0;
    while (sc_hashmap_iter_next(&iter, &key, &val)) {
        count++;
    }
    assert(count == 10);

    sc_hashmap_destroy(map, NULL);
    printf("  PASS: iterator\n");
}

static void test_contains(void) {
    SCHashMap *map = sc_hashmap_create(sizeof(int), sizeof(int), NULL, NULL);

    int key = 7, value = 77;
    assert(!sc_hashmap_contains(map, &key));
    sc_hashmap_set(map, &key, &value);
    assert(sc_hashmap_contains(map, &key));

    int missing = 8;
    assert(!sc_hashmap_contains(map, &missing));

    sc_hashmap_destroy(map, NULL);
    printf("  PASS: contains\n");
}

static void test_clear(void) {
    SCHashMap *map = sc_hashmap_create(sizeof(int), sizeof(int), NULL, NULL);

    for (int i = 0; i < 50; i++) {
        sc_hashmap_set(map, &i, &i);
    }
    assert(sc_hashmap_count(map) == 50);

    sc_hashmap_clear(map, NULL);
    assert(sc_hashmap_count(map) == 0);

    /* Can still use after clear */
    int key = 1, val = 2;
    sc_hashmap_set(map, &key, &val);
    assert(sc_hashmap_count(map) == 1);

    sc_hashmap_destroy(map, NULL);
    printf("  PASS: clear\n");
}

int main(void) {
    printf("test_hashmap:\n");
    test_create_destroy();
    test_set_get();
    test_overwrite();
    test_remove();
    test_many_entries();
    test_iterator();
    test_contains();
    test_clear();
    printf("All hashmap tests passed!\n");
    return 0;
}
