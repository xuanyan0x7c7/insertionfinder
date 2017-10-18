#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "../utils/memory.h"
#include "algorithm.h"


static int compare(const void* p, const void* q);


bool algorithm_contains_formula(
    const Algorithm* algorithm,
    const Formula* formula
) {
    const Formula* begin = algorithm->formula_list;
    const Formula* end = begin + algorithm->size;
    for (const Formula* p = begin; p < end; ++p) {
        if (formula_compare(p, formula) == 0) {
            return true;
        }
    }
    return false;
}

void algorithm_add_formula(Algorithm* algorithm, const Formula* formula) {
    if (algorithm_contains_formula(algorithm, formula)) {
        return;
    }
    if (algorithm->size == algorithm->capacity) {
        REALLOC(algorithm->formula_list, Formula, algorithm->capacity <<= 1);
    }
    Formula* new_formula = &algorithm->formula_list[algorithm->size];
    formula_duplicate(new_formula, formula);
    new_formula->begin_mask = formula->begin_mask;
    new_formula->end_mask = formula->end_mask;
    new_formula->set_up_mask = formula->set_up_mask;
    ++algorithm->size;
}

void algorithm_sort_formula(Algorithm* algorithm) {
    qsort(
        algorithm->formula_list,
        algorithm->size,
        sizeof(Formula),
        compare
    );
}


int compare(const void* p, const void* q) {
    return formula_compare((const Formula*)p, (const Formula*)q);
}
