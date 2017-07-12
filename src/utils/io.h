#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bool SafeRead(void* p, size_t size, size_t n, FILE* stream);
bool SafeWrite(const void* p, size_t size, size_t n, FILE* stream);
