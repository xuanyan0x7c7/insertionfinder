#pragma once
#include "utils.h"

typedef struct LinkedListNode LinkedListNode;
struct LinkedListNode {
    void* data;
    LinkedListNode* prev;
    LinkedListNode* next;
};

typedef struct LinkedList LinkedList;
struct LinkedList {
    LinkedListNode* head;
    LinkedListNode* tail;
    Destructor* destroy;
};

void linkedlist_construct(LinkedList* list, Destructor* destroy);
void linkedlist_destroy(LinkedList* list);

void linkedlist_insert_before(LinkedListNode* node, void* data);
void linkedlist_insert_after(LinkedListNode* node, void* data);
void linkedlist_remove(LinkedList* list, LinkedListNode* node);

void linkedlist_set_item(
    LinkedList* list,
    LinkedListNode* node,
    void* data
);
