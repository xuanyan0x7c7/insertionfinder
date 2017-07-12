#include <stdlib.h>
#include <string.h>
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "../utils/memory.h"
#include "../utils/io.h"
#include "algorithm.h"


static int CubeCompare(const Cube* x, const Cube* y);


void AlgorithmConstruct(Algorithm* algorithm, const Cube* state) {
    memcpy(&algorithm->state, state, sizeof(Cube));
    algorithm->mask = CubeMask(state);
    algorithm->corner_cycles = CubeCornerCycles(state);
    algorithm->edge_cycles = CubeEdgeCycles(state);
    algorithm->size = 0;
    algorithm->capacity = 8;
    algorithm->formula_list = MALLOC(Formula, algorithm->capacity);
}

void AlgorithmDestroy(Algorithm* algorithm) {
    Formula* begin = algorithm->formula_list;
    Formula* end = begin + algorithm->size;
    for (Formula* p = begin; p < end; ++p) {
        FormulaDestroy(p);
    }
    free(algorithm->formula_list);
}


void AlgorithmSave(const Algorithm* algorithm, FILE* stream) {
    CubeSave(&algorithm->state, stream);
    fwrite(&algorithm->size, sizeof(size_t), 1, stream);
    const Formula* begin = algorithm->formula_list;
    const Formula* end = begin + algorithm->size;
    for (const Formula* p = begin; p < end; ++p) {
        FormulaSave(p, stream);
    }
}

bool AlgorithmLoad(Algorithm* algorithm, FILE* stream) {
    Cube* state = &algorithm->state;
    if (!CubeLoad(state, stream)) {
        return false;
    }
    algorithm->mask = CubeMask(state);
    algorithm->corner_cycles = CubeCornerCycles(state);
    algorithm->edge_cycles = CubeEdgeCycles(state);
    size_t size;
    if (!SafeRead(&size, sizeof(size_t), 1, stream)) {
        return false;
    }
    algorithm->size = size;
    algorithm->capacity = size;
    algorithm->formula_list = MALLOC(Formula, size);
    Formula* begin = algorithm->formula_list;
    Formula* end = begin + algorithm->size;
    for (Formula* p = begin; p < end; ++p) {
        if (!FormulaLoad(p, stream)) {
            return false;
        }
    }
    return true;
}


int AlgorithmCompare(const Algorithm* x, const Algorithm* y) {
    int cycles_diff = (x->corner_cycles + x->edge_cycles) - (
        y->corner_cycles + y->edge_cycles
    );
    if (cycles_diff) {
        return cycles_diff;
    }
    if (x->corner_cycles != y->corner_cycles) {
        return x->corner_cycles - y->corner_cycles;
    }
    return CubeCompare(&x->state, &y->state);
}


int CubeCompare(const Cube* x, const Cube* y) {
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
