#pragma once
#include <stdlib.h>

#define MALLOC_HELPER(T, n, ...) (T*)safe_malloc((n) * sizeof(T))
#define MALLOC(...) MALLOC_HELPER(__VA_ARGS__, 1, 1)
#define REALLOC(p, T, n) p = (T*)safe_realloc(p, (n) * sizeof(T))

void* safe_malloc(size_t size);
void* safe_realloc(void* p, size_t size);
