#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "cube.h"
#include "formula.h"

typedef struct Algorithm Algorithm;
struct Algorithm {
    Cube state;
    unsigned mask;
    bool corner_changed;
    bool edge_changed;
    size_t size;
    size_t capacity;
    Formula* formula_list;
};

Algorithm* AlgorithmConstruct(Algorithm* algorithm, const Cube* state);
void AlgorithmDestroy(Algorithm* algorithm);

void AlgorithmSave(const Algorithm* algorithm, FILE* stream);
Algorithm* AlgorithmLoad(Algorithm* algorithm, FILE* stream);

size_t AlgorithmSize(const Algorithm* algorithm);
const Formula* AlgorithmGetFormulaList(const Algorithm* algorithm);
void AlgorithmAddFormula(Algorithm* algorithm, const Formula* formula);