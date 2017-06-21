#include <stdlib.h>
#include "linked-list.h"

typedef LinkedList List;
typedef LinkedListNode Node;

static inline void Noop(void* data) {}

List* LinkedListConstruct(List* list, Destructor destructor) {
    if (!list) {
        list = (List*)malloc(sizeof(List));
    }
    list->head = (Node*)malloc(sizeof(Node));
    list->tail = (Node*)malloc(sizeof(Node));
    list->head->data = NULL;
    list->head->prev = NULL;
    list->head->next = list->tail;
    list->tail->data = NULL;
    list->tail->prev = list->head;
    list->tail->next = NULL;
    list->destroy = destructor ? destructor : Noop;
    return list;
}

void LinkedListDestroy(List* list) {
    Node* node = list->head;
    Node* next = node->next;
    while ((node = next) != list->tail) {
        next = next->next;
        list->destroy(node->data);
        free(node);
    }
    free(list->head);
    free(list->tail);
}

void LinkedListInsertBefore(Node* node, void* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->prev = node->prev;
    new_node->next = node;
    node->prev->next = new_node;
    node->prev = new_node;
}

void LinkedListInsertAfter(Node* node, void* data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->prev = node;
    new_node->next = node->next;
    node->next->prev = new_node;
    node->next = new_node;
}

void LinkedListRemove(List* list, Node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->destroy(node->data);
    free(node);
}

void LinkedListSetItem(List* list, Node* node, void* data) {
    list->destroy(node->data);
    node->data = data;
}
