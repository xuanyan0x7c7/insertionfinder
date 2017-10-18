#include <stdlib.h>
#include <string.h>
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "../utils/memory.h"
#include "../utils/io.h"
#include "algorithm.h"


static int cube_compare(const Cube* x, const Cube* y);


void algorithm_construct(Algorithm* algorithm, const Cube* state) {
    memcpy(&algorithm->state, state, sizeof(Cube));
    algorithm->mask = cube_mask(state);
    algorithm->parity = cube_has_parity(state);
    algorithm->corner_cycles = cube_corner_cycles(state);
    algorithm->edge_cycles = cube_edge_cycles(state);
    algorithm->size = 0;
    algorithm->capacity = 8;
    algorithm->formula_list = MALLOC(Formula, algorithm->capacity);
}

void algorithm_destroy(Algorithm* algorithm) {
    Formula* begin = algorithm->formula_list;
    Formula* end = begin + algorithm->size;
    for (Formula* p = begin; p < end; ++p) {
        formula_destroy(p);
    }
    free(algorithm->formula_list);
}


void algorithm_save(const Algorithm* algorithm, FILE* stream) {
    cube_save(&algorithm->state, stream);
    fwrite(&algorithm->size, sizeof(size_t), 1, stream);
    const Formula* begin = algorithm->formula_list;
    const Formula* end = begin + algorithm->size;
    for (const Formula* p = begin; p < end; ++p) {
        formula_save(p, stream);
    }
}

bool algorithm_load(Algorithm* algorithm, FILE* stream) {
    Cube* state = &algorithm->state;
    if (!cube_load(state, stream)) {
        return false;
    }
    algorithm->mask = cube_mask(state);
    algorithm->parity = cube_has_parity(state);
    algorithm->corner_cycles = cube_corner_cycles(state);
    algorithm->edge_cycles = cube_edge_cycles(state);
    size_t size;
    if (!safe_read(&size, sizeof(size_t), 1, stream)) {
        return false;
    }
    algorithm->size = size;
    algorithm->capacity = size;
    algorithm->formula_list = MALLOC(Formula, size);
    Formula* begin = algorithm->formula_list;
    Formula* end = begin + algorithm->size;
    for (Formula* p = begin; p < end; ++p) {
        if (!formula_load(p, stream)) {
            return false;
        }
    }
    return true;
}


int algorithm_compare(const Algorithm* x, const Algorithm* y) {
    int cycles_diff = (x->corner_cycles + x->edge_cycles + x->parity) - (
        y->corner_cycles + y->edge_cycles + y->parity
    );
    if (cycles_diff) {
        return cycles_diff;
    }
    if (x->parity != y->parity) {
        return x->parity - y->parity;
    }
    if (x->corner_cycles != y->corner_cycles) {
        return x->corner_cycles - y->corner_cycles;
    }
    return cube_compare(&x->state, &y->state);
}


int cube_compare(const Cube* x, const Cube* y) {
    const int* corner1 = x->corner;
    const int* corner2 = y->corner;
    const int* edge1 = x->edge;
    const int* edge2 = y->edge;
    for (int i = 0; i < 8; ++i) {
        if (corner1[i] != corner2[i]) {
            return corner1[i] - corner2[i];
        }
    }
    for (int i = 0; i < 12; ++i) {
        if (edge1[i] != edge2[i]) {
            return edge1[i] - edge2[i];
        }
    }
    return 0;
}
