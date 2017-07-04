#pragma once
#include <stddef.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../formula/formula.h"

typedef struct Finder Finder;
typedef struct FinderWorker FinderWorker;
typedef struct Insertion Insertion;

struct Finder {
    size_t algorithm_count;
    Algorithm** algorithm_list;
    int corner_cycle_index[6 * 24 * 24];
    int edge_cycle_index[10 * 24 * 24];
    bool change_corner;
    bool change_edge;
    Formula scramble;
    Cube scramble_cube;
    Cube inverse_scramble_cube;
    size_t fewest_moves;
    size_t solution_count;
    size_t solution_capacity;
    FinderWorker* solution_list;
};

struct FinderWorker {
    Finder* finder;
    size_t depth;
    size_t solving_step_capacity;
    Insertion* solving_step;
    size_t cancellation;
};

struct Insertion {
    Formula skeleton;
    size_t insert_place;
    const Formula *insertion;
};

void FinderConstruct(
    Finder* finder,
    size_t algorithm_count,
    Algorithm** algorithm_list,
    const Formula* scramble
);
void FinderDestroy(Finder* finder);

void FinderSolve(Finder* finder, const Formula* skeleton);

void FinderWorkerConstruct(
    FinderWorker* worker,
    Finder* finder,
    const Formula* skeleton
);
void FinderWorkerDestroy(FinderWorker* worker);

void FinderWorkerSearch(
    FinderWorker* worker,
    int corner_cycles, int edge_cycles,
    size_t begin, size_t end
);
