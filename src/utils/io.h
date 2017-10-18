#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bool safe_read(void* p, size_t size, size_t n, FILE* stream);
bool safe_write(const void* p, size_t size, size_t n, FILE* stream);
