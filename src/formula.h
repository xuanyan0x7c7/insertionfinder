#pragma once
#include <stddef.h>

typedef struct Formula Formula;
struct Formula {
    size_t length;
    size_t capacity;
    int* move;
};

Formula* FormulaConstruct(Formula* formula, const char* string);
void FormulaDestroy(Formula* formula);

char* FormulaToString(const Formula* formula, char* string);

size_t FormulaCancelMoves(Formula* formula);
