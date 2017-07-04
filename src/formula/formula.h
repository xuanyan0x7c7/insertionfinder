#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

enum {
    TWIST_U0, TWIST_U, TWIST_U2, TWIST_U3,
    TWIST_D0, TWIST_D, TWIST_D2, TWIST_D3,
    TWIST_R0, TWIST_R, TWIST_R2, TWIST_R3,
    TWIST_L0, TWIST_L, TWIST_L2, TWIST_L3,
    TWIST_F0, TWIST_F, TWIST_F2, TWIST_F3,
    TWIST_B0, TWIST_B, TWIST_B2, TWIST_B3
};

static const int inverse_move_table[] = {
    TWIST_U0, TWIST_U3, TWIST_U2, TWIST_U,
    TWIST_D0, TWIST_D3, TWIST_D2, TWIST_D,
    TWIST_R0, TWIST_R3, TWIST_R2, TWIST_R,
    TWIST_L0, TWIST_L3, TWIST_L2, TWIST_L,
    TWIST_F0, TWIST_F3, TWIST_F2, TWIST_F,
    TWIST_B0, TWIST_B3, TWIST_B2, TWIST_B
};

typedef struct Formula Formula;
struct Formula {
    size_t length;
    size_t capacity;
    int* move;
};

bool FormulaConstruct(Formula* formula, const char* string);
void FormulaDestroy(Formula* formula);

void FormulaSave(const Formula* formula, FILE* stream);
Formula* FormulaLoad(Formula* formula, FILE* stream);
Formula* FormulaDuplicate(Formula* formula, const Formula* source);

void FormulaPrint(const Formula* formula, FILE* stream);
void FormulaPrintRange(
    const Formula* formula,
    size_t begin, size_t end,
    FILE* stream
);

size_t FormulaCancelMoves(Formula* formula);

size_t FormulaInsert(
    const Formula* formula,
    size_t insert_place,
    const Formula* insertion,
    Formula* result
);

bool FormulaSwappable(const Formula* formula, size_t index);
void FormulaSwapAdjacent(Formula* formula, size_t index);
void FormulaNormalize(Formula* formula);
int FormulaCompare(const Formula* f1, const Formula* f2);

size_t FormulaGenerateIsomorphisms(const Formula* formula, Formula* result);
