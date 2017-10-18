#include <stdio.h>
#include <stdlib.h>
#include "memory.h"


void* safe_malloc(size_t size) {
    void* p = malloc(size);
    if (p) {
        return p;
    } else {
        perror("");
        exit(EXIT_FAILURE);
    }
}

void* safe_realloc(void* p, size_t size) {
    p = realloc(p, size);
    if (p) {
        return p;
    } else {
        perror("");
        exit(EXIT_FAILURE);
    }
}
