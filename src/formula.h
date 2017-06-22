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

size_t FormulaLength(const Formula* formula);
int FormulaGetMove(const Formula* formula, size_t index);
void FormulaSetMove(Formula* formula, size_t index, int move);

char* FormulaToString(const Formula* formula, char* string);

size_t FormulaCancelMoves(Formula* formula);
