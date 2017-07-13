#include <stdio.h>
#include <stdlib.h>
#include "memory.h"


void* SafeMalloc(size_t size) {
    void* p = malloc(size);
    if (p) {
        return p;
    } else {
        perror("");
        exit(EXIT_FAILURE);
    }
}

void* SafeRealloc(void* p, size_t size) {
    p = realloc(p, size);
    if (p) {
        return p;
    } else {
        perror("");
        exit(EXIT_FAILURE);
    }
}
