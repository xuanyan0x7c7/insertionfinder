#pragma once
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../formula/formula.h"


enum FinderSolveStatus {
    SOLVE_SUCCESS,
    SOLVE_SUCCESS_SOLVED,
    SOLVE_FAILURE_PARITY_ALGORITHMS_NEEDED,
    SOLVE_FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED,
    SOLVE_FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED
};
typedef enum FinderSolveStatus FinderSolveStatus;

typedef struct FinderSolveResult FinderSolveResult;
struct FinderSolveResult {
    FinderSolveStatus status;
    int64_t duration;
    int64_t cpu_time;
};

typedef struct Finder Finder;
typedef struct FinderWorker FinderWorker;
typedef struct Insertion Insertion;

struct Finder {
    size_t algorithm_count;
    Algorithm** algorithm_list;
    int parity_index[7 * 24 * 11 * 24];
    int corner_cycle_index[6 * 24 * 24];
    int edge_cycle_index[10 * 24 * 24];
    bool change_parity;
    bool change_corner;
    bool change_edge;
    Formula scramble;
    Cube scramble_cube;
    Cube inverse_scramble_cube;
    size_t fewest_moves;
    size_t solution_count;
    size_t solution_capacity;
    FinderWorker* solution_list;
    pthread_mutex_t mutex;
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

void finder_construct(
    Finder* finder,
    size_t algorithm_count,
    Algorithm** algorithm_list,
    const Formula* scramble
);
void finder_destroy(Finder* finder);

FinderSolveResult finder_solve(
    Finder* finder,
    const Formula* skeleton,
    size_t max_threads
);

void finder_worker_construct(
    FinderWorker* worker,
    Finder* finder,
    const Formula* skeleton
);
void finder_worker_destroy(FinderWorker* worker);

void finder_worker_search(
    FinderWorker* worker,
    bool parity, int corner_cycles, int edge_cycles,
    size_t begin, size_t end
);
