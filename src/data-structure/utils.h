#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef void Destructor(void*);
typedef bool EqualFunction(const void*, const void*);
typedef size_t HashFunction(const void*);
