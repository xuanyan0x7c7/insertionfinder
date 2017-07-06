#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "finder.h"


typedef FinderWorker Worker;

static int SolutionCompare(const void* p, const void* q);


void FinderConstruct(
    Finder* finder,
    size_t algorithm_count, const Algorithm** algorithm_list,
    const Formula* scramble
) {
    finder->algorithm_count = algorithm_count;
    finder->algorithm_list = algorithm_list;
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
    CubeConstruct(&finder->scramble_cube);
    CubeTwistFormula(&finder->scramble_cube, scramble, true, true, false);
    CubeInverseState(&finder->scramble_cube, &finder->inverse_scramble_cube);

    finder->fewest_moves = ULONG_MAX;
    finder->solution_count = 0;
    finder->solution_capacity = 16;
    finder->solution_list = (Worker*)malloc(
        finder->solution_capacity * sizeof(Worker)
    );
}

void FinderDestroy(Finder* finder) {
    FormulaDestroy(&finder->scramble);
    Worker* begin = finder->solution_list;
    Worker* end = begin + finder->solution_count;
    for (Worker* p = begin; p < end; ++p) {
        FinderWorkerDestroy(p);
    }
    free(finder->solution_list);
}


FinderSolveStatus FinderSolve(Finder* finder, const Formula* skeleton) {
    Cube cube = identity_cube;
    CubeTwistFormula(&cube, &finder->scramble, true, true, false);
    CubeTwistFormula(&cube, skeleton, true, true, false);
    int corner_cycles = CubeCornerCycles(&cube);
    int edge_cycles = CubeEdgeCycles(&cube);
    if (!corner_cycles && !edge_cycles) {
        return SOLVE_SUCCESS_SOLVED;
    } else if (corner_cycles && !finder->change_corner) {
        return SOLVE_FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED;
    } else if (edge_cycles && !finder->change_edge) {
        return SOLVE_FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED;
    }
    FinderWorker worker;
    FinderWorkerConstruct(&worker, finder, skeleton);
    FinderWorkerSearch(
        &worker,
        corner_cycles, edge_cycles,
        0, skeleton->length
    );
    FinderWorkerDestroy(&worker);
    Worker* begin = finder->solution_list;
    Worker* end = begin + finder->solution_count;
    for (Worker* solution = begin; solution < end; ++solution) {
        solution->cancellation = solution->solving_step[0].skeleton.length;
        for (size_t j = 0; j < solution->depth; ++j) {
            solution->cancellation += solution->solving_step[j]
                .insertion->length;
        }
        solution->cancellation -= solution->solving_step[solution->depth]
            .skeleton.length;
    }
    qsort(
        finder->solution_list,
        finder->solution_count,
        sizeof(Worker),
        SolutionCompare
    );

    return SOLVE_SUCCESS;
}


int SolutionCompare(const void* p, const void* q) {
    return ((const FinderWorker*)p)->cancellation
        - ((const FinderWorker*)q)->cancellation;
}
