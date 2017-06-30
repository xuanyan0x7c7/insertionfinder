#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "hash-map.h"


static void Noop(void* data) {}


HashMap* HashMapConstruct(
    HashMap* map,
    EqualFunction* key_equal,
    HashFunction* hash,
    Destructor* destroy_key,
    Destructor* destroy_value
) {
    if (!map) {
        map = (HashMap*)malloc(sizeof(HashMap));
    }
    map->size = 0;
    map->buckets = 256;
    map->buffer = (HashMapNode*)malloc(map->buckets * sizeof(HashMapNode));
    for (size_t i = 0; i < map->buckets; ++i) {
        map->buffer[i].status = NODE_EMPTY;
    }
    map->key_equal = key_equal;
    map->hash = hash;
    map->destroy_key = destroy_key ? destroy_key : Noop;
    map->destroy_value = destroy_value ? destroy_value : Noop;
    return map;
}

void HashMapDestroy(HashMap* map) {
    for (size_t i = 0; i < map->buckets; ++i) {
        if (map->buffer[i].status == NODE_FILLED) {
            map->destroy_key(map->buffer[i].key);
            map->destroy_value(map->buffer[i].value);
        }
    }
    free(map->buffer);
}


HashMapNode* HashMapFind(const HashMap* map, const void* key) {
    size_t index = map->hash(key) & (map->buckets - 1);
    while (map->buffer[index].status != NODE_EMPTY) {
        if (
            map->buffer[index].status == NODE_FILLED
            && map->key_equal(map->buffer[index].key, key)
        ) {
            return &map->buffer[index];
        }
        index = (index + 1) & (map->buckets - 1);
    }
    return NULL;
}

HashMapNode* HashMapInsert(HashMap* map, void* key, void* value) {
    size_t buckets_mask = map->buckets - 1;
    size_t index = map->hash(key) & buckets_mask;
    while (map->buffer[index].status == NODE_FILLED) {
        if (map->key_equal(map->buffer[index].key, key)) {
            return NULL;
        } else {
            index = (index + 1) & buckets_mask;
        }
    }

    if (map->size == (map->buckets >> 2) * 3) {
        size_t new_buckets = map->buckets << 1;
        size_t buckets_mask = new_buckets - 1;
        HashMapNode* new_buffer = (HashMapNode*)malloc(
            new_buckets * sizeof(HashMapNode)
        );
        for (size_t i = 0; i < new_buckets; ++i) {
            new_buffer[i].status = NODE_EMPTY;
        }
        for (size_t i = 0; i < map->buckets; ++i) {
            size_t index = map->hash(map->buffer[i].key) & buckets_mask;
            while (new_buffer[index].status == NODE_FILLED) {
                index = (index + 1) & buckets_mask;
            }
            new_buffer[index].key = map->buffer[i].key;
            new_buffer[index].value = map->buffer[i].value;
        }
        free(map->buffer);
        map->buckets = new_buckets;
        map->buffer = new_buffer;
        size_t index = map->hash(key) & buckets_mask;
        while (map->buffer[index].status == NODE_FILLED) {
            index = (index + 1) & buckets_mask;
        }
    }

    ++map->size;
    map->buffer[index].status = NODE_FILLED;
    map->buffer[index].key = key;
    map->buffer[index].value = value;
    return &map->buffer[index];
}

void HashMapRemove(HashMap* map, HashMapNode* node) {
    --map->size;
    node->status = NODE_REMOVED;
    map->destroy_key(node->key);
    map->destroy_value(node->value);
    if (map->size == (map->buckets >> 3) * 3 && map->buckets > 256) {
        size_t new_buckets = map->buckets >> 1;
        size_t buckets_mask = new_buckets - 1;
        HashMapNode* new_buffer = (HashMapNode*)malloc(
            new_buckets * sizeof(HashMapNode)
        );
        for (size_t i = 0; i < new_buckets; ++i) {
            new_buffer[i].status = NODE_EMPTY;
        }
        for (size_t i = 0; i < map->buckets; ++i) {
            size_t index = map->hash(map->buffer[i].key) & buckets_mask;
            while (new_buffer[index].status == NODE_FILLED) {
                index = (index + 1) & buckets_mask;
            }
            new_buffer[index].key = map->buffer[i].key;
            new_buffer[index].value = map->buffer[i].value;
        }
    }
}


HashMapNode* HashMapIterStart(const HashMap* map) {
    const HashMapNode* begin = map->buffer;
    const HashMapNode* end = map->buffer + map->buckets;
    for (HashMapNode* iter = (HashMapNode*)begin; iter != end; ++iter) {
        if (iter->status == NODE_FILLED) {
            return iter;
        }
    }
    return NULL;
}

HashMapNode* HashMapIterNext(const HashMap* map, const HashMapNode* node) {
    const HashMapNode* end = map->buffer + map->buckets;
    for (HashMapNode* iter = (HashMapNode*)node; iter != end; ++iter) {
        if (iter->status == NODE_FILLED) {
            return iter;
        }
    }
    return NULL;
}
