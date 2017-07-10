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
    const Formula* begin = algorithm->formula_list;
    const Formula* end = begin + algorithm->size;
    for (const Formula* p = begin; p < end; ++p) {
        if (FormulaCompare(p, formula) == 0) {
            return true;
        }
    }
    return false;
}

void AlgorithmAddFormula(Algorithm* algorithm, const Formula* formula) {
    if (AlgorithmContainsFormula(algorithm, formula)) {
        return;
    }
    if (algorithm->size == algorithm->capacity) {
        algorithm->formula_list = (Formula*)realloc(
            algorithm->formula_list,
            (algorithm->capacity <<= 1) * sizeof(Formula)
        );
    }
    Formula* new_formula = &algorithm->formula_list[algorithm->size];
    FormulaDuplicate(new_formula, formula);
    new_formula->begin_mask = formula->begin_mask;
    new_formula->end_mask = formula->end_mask;
    new_formula->set_up_mask = formula->set_up_mask;
    ++algorithm->size;
}

void AlgorithmSortFormula(Algorithm* algorithm) {
    qsort(
        algorithm->formula_list,
        algorithm->size,
        sizeof(Formula),
        FormulaCompareGeneric
    );
}


int FormulaCompareGeneric(const void* p, const void* q) {
    return FormulaCompare((const Formula*)p, (const Formula*)q);
}
