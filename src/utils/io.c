#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "io.h"


bool safe_read(void* p, size_t size, size_t n, FILE* stream) {
    size_t bytes = fread(p, size, n, stream);
    if (bytes == n) {
        return true;
    } else {
        perror("");
        return false;
    }
}

bool safe_write(const void* p, size_t size, size_t n, FILE* stream) {
    size_t bytes = fwrite(p, size, n, stream);
    if (bytes == n) {
        return true;
    } else {
        perror("");
        return false;
    }
}
