#pragma once
#include <stdio.h>

char* Trim(const char* string);
char* GetLine(FILE* stream);

bool CubeEqualGeneric(const void* p, const void* q);
size_t CubeHashGeneric(const void* p);
int AlgorithmCompareGeneric(const void* p, const void* q);
void AlgorithmFreeGeneric(void* p);
