#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "algorithm.h"


static int FormulaCompareGeneric(const void* p, const void* q);


bool AlgorithmContainsFormula(
    const Algorithm* algorithm,
    const Formula* formula
) {
    return bsearch(
        formula,
        algorithm->formula_list,
        algorithm->size, sizeof(Formula),
        FormulaCompareGeneric
    );
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
        FormulaDuplicate(pointer + 1, formula);
    }
    ++algorithm->size;
}


int FormulaCompareGeneric(const void* p, const void* q) {
    return FormulaCompare((const Formula*)p, (const Formula*)q);
}
