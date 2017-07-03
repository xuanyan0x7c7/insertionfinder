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

HashMap* HashMapConstruct(
    HashMap* map,
    EqualFunction* key_equal,
    HashFunction* hash,
    Destructor* destroy_key,
    Destructor* destroy_value
);
void HashMapDestroy(HashMap* map);

HashMapNode* HashMapFind(const HashMap* map, const void* key);
bool HashMapInsert(HashMap* map, void* key, void* value, HashMapNode** node);
void HashMapRemove(HashMap* map, HashMapNode* node);

HashMapNode* HashMapIterStart(const HashMap* map);
HashMapNode* HashMapIterNext(const HashMap* map, const HashMapNode* node);
