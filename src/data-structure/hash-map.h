#pragma once
#include <stddef.h>
#include "utils.h"

enum HashMapNodeStatus {
    NODE_EMPTY, NODE_FILLED, NODE_REMOVED
};
typedef enum HashMapNodeStatus HashMapNodeStatus;

typedef struct HashMapNode HashMapNode;
struct HashMapNode {
    HashMapNodeStatus status;
    void* key;
    void* value;
};

typedef struct HashMap HashMap;
struct HashMap {
    size_t size;
    size_t buckets;
    HashMapNode* buffer;
    EqualFunction* key_equal;
    HashFunction* hash;
    Destructor* destroy_key;
    Destructor* destroy_value;
};

void hashmap_construct(
    HashMap* map,
    EqualFunction* key_equal,
    HashFunction* hash,
    Destructor* destroy_key,
    Destructor* destroy_value
);
void hashmap_destroy(HashMap* map);

HashMapNode* hashmap_find(const HashMap* map, const void* key);
bool hashmap_insert(HashMap* map, void* key, void* value, HashMapNode** node);
void hashmap_remove(HashMap* map, HashMapNode* node);

HashMapNode* hashmap_iter_start(const HashMap* map);
HashMapNode* hashmap_iter_next(const HashMap* map, const HashMapNode* node);
