#include <stdlib.h>
#include <string.h>
#include "cube.h"
#include "formula.h"
#include "algorithm.h"


static int FormulaCompareGeneric(const void* p, const void* q);
static int CubeCompare(const Cube* x, const Cube* y);


Algorithm* AlgorithmConstruct(Algorithm* algorithm, const Cube* state) {
    if (!algorithm) {
        algorithm = (Algorithm*)malloc(sizeof(Algorithm));
    }
    memcpy(&algorithm->state, state, sizeof(Cube));
    algorithm->mask = CubeMask(state);
    algorithm->corner_cycles = CubeCornerCycles(state);
    algorithm->edge_cycles = CubeEdgeCycles(state);
    algorithm->size = 0;
    algorithm->capacity = 8;
    algorithm->formula_list = (Formula*)malloc(
        algorithm->capacity * sizeof(Formula)
    );
    return algorithm;
}

void AlgorithmDestroy(Algorithm* algorithm) {
    for (size_t i = 0; i < algorithm->size; ++i) {
        FormulaDestroy(&algorithm->formula_list[i]);
    }
    free(algorithm->formula_list);
    algorithm->formula_list = NULL;
}


void AlgorithmSave(const Algorithm* algorithm, FILE* stream) {
    CubeSave(&algorithm->state, stream);
    fwrite(&algorithm->size, sizeof(size_t), 1, stream);
    for (size_t i = 0; i < algorithm->size; ++i) {
        FormulaSave(&algorithm->formula_list[i], stream);
    }
}

Algorithm* AlgorithmLoad(Algorithm* algorithm, FILE* stream) {
    if (!algorithm) {
        algorithm = (Algorithm*)malloc(sizeof(Algorithm));
    }
    Cube* state = &algorithm->state;
    CubeLoad(state, stream);
    algorithm->mask = CubeMask(state);
    algorithm->corner_cycles = CubeCornerCycles(state);
    algorithm->edge_cycles = CubeEdgeCycles(state);
    size_t size;
    fread(&size, sizeof(size_t), 1, stream);
    algorithm->size = size;
    algorithm->capacity = size;
    algorithm->formula_list = (Formula*)realloc(
        algorithm->formula_list,
        size * sizeof(Formula)
    );
    for (size_t i = 0; i < algorithm->size; ++i) {
        FormulaLoad(&algorithm->formula_list[i], stream);
    }
    return algorithm;
}


size_t AlgorithmSize(const Algorithm* algorithm) {
    return algorithm->size;
}

const Formula* AlgorithmGetFormulaList(const Algorithm* algorithm) {
    return algorithm->formula_list;
}

void AlgorithmAddFormula(Algorithm* algorithm, const Formula* formula) {
    if (algorithm->size == algorithm->capacity) {
        algorithm->formula_list = (Formula*)realloc(
            algorithm->formula_list,
            (algorithm->capacity <<= 1) * sizeof(Formula)
        );
    }
    if (!bsearch(
        formula,
        algorithm->formula_list,
        algorithm->size, sizeof(Formula),
        FormulaCompareGeneric
    )) {
        Formula* pointer = algorithm->formula_list + algorithm->size;
        while (--pointer >= algorithm->formula_list) {
            if (FormulaCompare(pointer, formula) > 0) {
                memcpy(pointer + 1, pointer, sizeof(Formula));
            } else {
                break;
            }
        }
        FormulaConstruct(pointer + 1, NULL);
        FormulaDuplicate(pointer + 1, formula);
    }
    ++algorithm->size;
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


int FormulaCompareGeneric(const void* p, const void* q) {
    return FormulaCompare((const Formula*)p, (const Formula*)q);
}

int CubeCompare(const Cube* x, const Cube* y) {
    const int* corner1 = x->corner;
    const int* corner2 = y->corner;
    const int* edge1 = x->edge;
    const int* edge2 = y->edge;
    for (int i = 0; i < 8; ++i) {
        if (corner1[i] != corner2[i]) {
            return false;
        }
    }
    for (int i = 0; i < 12; ++i) {
        if (edge1[i] != edge2[i]) {
            return false;
        }
    }
    return true;
}
