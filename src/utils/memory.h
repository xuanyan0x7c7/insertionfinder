#pragma once
#include <stdlib.h>

#define MALLOC_HELPER(T, n, ...) (T*)SafeMalloc((n) * sizeof(T))
#define MALLOC(...) MALLOC_HELPER(__VA_ARGS__, 1, 1)
#define REALLOC(p, T, n) p = (T*)SafeRealloc(p, (n) * sizeof(T))

void* SafeMalloc(size_t size);
void* SafeRealloc(void* p, size_t size);
