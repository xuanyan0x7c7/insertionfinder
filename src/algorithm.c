#include <stdlib.h>
#include <string.h>
#include "cube.h"
#include "formula.h"
#include "algorithm.h"


Algorithm* AlgorithmConstruct(Algorithm* algorithm, const Cube* state) {
    if (!algorithm) {
        algorithm = (Algorithm*)malloc(sizeof(Algorithm));
    }
    memcpy(&algorithm->state, state, sizeof(Cube));
    algorithm->mask = CubeMask(state);
    algorithm->corner_changed = algorithm->mask >> 12;
    algorithm->edge_changed = algorithm->mask & ((1 << 12) - 1);
    algorithm->size = 0;
    algorithm->capacity = 8;
    algorithm->formula_list = (Formula*)malloc(
        algorithm->capacity * sizeof(Formula)
    );
    return algorithm;
}

void AlgorithmDestroy(Algorithm* algorithm) {
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
    CubeLoad(&algorithm->state, stream);
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
    memcpy(
        &algorithm->formula_list[algorithm->size++],
        formula,
        sizeof(Formula)
    );
}
