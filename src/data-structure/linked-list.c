#include <stdlib.h>
#include "../utils/memory.h"
#include "linked-list.h"


typedef LinkedList List;
typedef LinkedListNode Node;

static void noop(void* data) {}


void linkedlist_construct(List* list, Destructor* destroy) {
    list->head = MALLOC(Node);
    list->tail = MALLOC(Node);
    list->head->data = NULL;
    list->head->prev = NULL;
    list->head->next = list->tail;
    list->tail->data = NULL;
    list->tail->prev = list->head;
    list->tail->next = NULL;
    list->destroy = destroy ? destroy : noop;
}

void linkedlist_destroy(List* list) {
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


void linkedlist_insert_before(Node* node, void* data) {
    Node* new_node = MALLOC(Node);
    new_node->data = data;
    new_node->prev = node->prev;
    new_node->next = node;
    node->prev->next = new_node;
    node->prev = new_node;
}

void linkedlist_insert_after(Node* node, void* data) {
    Node* new_node = MALLOC(Node);
    new_node->data = data;
    new_node->prev = node;
    new_node->next = node->next;
    node->next->prev = new_node;
    node->next = new_node;
}

void linkedlist_remove(List* list, Node* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->destroy(node->data);
    free(node);
}


void linkedlist_set_item(List* list, Node* node, void* data) {
    list->destroy(node->data);
    node->data = data;
}
