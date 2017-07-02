#include <limits.h>
#include <stdlib.h>
#include "../formula.h"
#include "finder.h"


Insertion* InsertionConstruct(Insertion* insertion, const Formula* formula) {
    if (!insertion) {
        insertion = (Insertion*)malloc(sizeof(Insertion));
    }
    FormulaDuplicate(&insertion->partial_solution, formula);
    return insertion;
}

void InsertionDestroy(Insertion* insertion) {
    FormulaDestroy(&insertion->partial_solution);
}
