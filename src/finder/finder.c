#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "../cube.h"
#include "../formula.h"
#include "finder.h"


Finder* FinderConstruct(
    Finder* finder,
    size_t algorithm_count, const Algorithm** algorithm_list,
    const Formula* scramble
) {
    if (!finder) {
        finder = (Finder*)malloc(sizeof(Finder));
    }

    finder->algorithm_count = algorithm_count;
    memcpy(
        finder->algorithm_list,
        algorithm_list,
        algorithm_count * sizeof(Algorithm*)
    );
    memset(finder->corner_cycle_index, -1, 6 * 24 * 24 * sizeof(int));
    memset(finder->edge_cycle_index, -1, 10 * 24 * 24 * sizeof(int));
    finder->change_corner = false;
    finder->change_edge = false;
    for (size_t i = 0; i < algorithm_count; ++i) {
        const Algorithm* algorithm = algorithm_list[i];
        const Cube* state = &algorithm->state;
        int corner_cycles = algorithm->corner_cycles;
        int edge_cycles = algorithm->edge_cycles;
        if (corner_cycles) {
            finder->change_corner = true;
            if (corner_cycles == 1 && edge_cycles == 0) {
                finder->corner_cycle_index[CubeCorner3CycleIndex(state)] = i;
            }
        }
        if (edge_cycles) {
            finder->change_edge = true;
            if (edge_cycles == 1 && corner_cycles == 0) {
                finder->edge_cycle_index[CubeEdge3CycleIndex(state)] = i;
            }
        }
    }

    FormulaConstruct(&finder->scramble, NULL);
    FormulaDuplicate(&finder->scramble, scramble);
    CubeConstruct(&finder->scramble_cube, scramble);
    CubeInverseState(&finder->inverse_scramble_cube, &finder->scramble_cube);

    finder->fewest_moves = ULONG_MAX;
    finder->solution_count = 0;
    finder->solution_capacity = 16;
    finder->solution_list = (FinderWorker*)malloc(
        finder->solution_capacity * sizeof(FinderWorker)
    );

    return finder;
}

void FinderDestroy(Finder* finder) {
    FormulaDestroy(&finder->scramble);
    for (size_t i = 0; i < finder->solution_count; ++i) {
        FinderWorkerDestroy(&finder->solution_list[i]);
    }
    free(finder->solution_list);
    finder->solution_list = NULL;
}
