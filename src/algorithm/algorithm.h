#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "../cube/cube.h"
#include "../formula/formula.h"

typedef struct Algorithm Algorithm;
struct Algorithm {
    Cube state;
    uint32_t mask;
    bool parity;
    int corner_cycles;
    int edge_cycles;
    size_t size;
    size_t capacity;
    Formula* formula_list;
};

void algorithm_construct(Algorithm* algorithm, const Cube* state);
void algorithm_destroy(Algorithm* algorithm);

void algorithm_save(const Algorithm* algorithm, FILE* stream);
bool algorithm_load(Algorithm* algorithm, FILE* stream);

bool algorithm_contains_formula(
    const Algorithm* algorithm,
    const Formula* formula
);
void algorithm_add_formula(Algorithm* algorithm, const Formula* formula);
void algorithm_sort_formula(Algorithm* algorithm);

int algorithm_compare(const Algorithm* x, const Algorithm* y);
