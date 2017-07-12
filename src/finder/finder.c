#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "../utils/memory.h"
#include "finder.h"


typedef FinderWorker Worker;

typedef struct WorkerThread WorkerThread;
struct WorkerThread {
    pthread_t thread_id;
    Finder* finder;
    const Formula* skeleton;
    Worker worker;
    int corner_cycles;
    int edge_cycles;
    size_t begin;
    size_t end;
};

static void* WorkerThreadStart(void* arg);

static int SolutionCompare(const void* p, const void* q);


void FinderConstruct(
    Finder* finder,
    size_t algorithm_count, Algorithm** algorithm_list,
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
    finder->solution_list = MALLOC(Worker, finder->solution_capacity);

    pthread_mutex_init(&finder->mutex, NULL);
}

void FinderDestroy(Finder* finder) {
    FormulaDestroy(&finder->scramble);
    Worker* begin = finder->solution_list;
    Worker* end = begin + finder->solution_count;
    for (Worker* p = begin; p < end; ++p) {
        FinderWorkerDestroy(p);
    }
    free(finder->solution_list);
    pthread_mutex_destroy(&finder->mutex);
}


FinderSolveStatus FinderSolve(
    Finder* finder,
    const Formula* skeleton,
    size_t max_threads
) {
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

    size_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (thread_count > skeleton->length) {
        thread_count = skeleton->length + 1;
    }
    if (thread_count > max_threads) {
        thread_count = max_threads;
    }
    size_t split_points[thread_count + 1];
    for (size_t i = 0; i <= thread_count; ++i) {
        split_points[thread_count - i] = (size_t)(
            (skeleton->length + 1) * (1 - sqrt((double)i / thread_count)) + 0.5
        );
    }
    WorkerThread worker_threads[thread_count];
    for (size_t i = 0; i < thread_count; ++i) {
        WorkerThread* thread = &worker_threads[i];
        thread->finder = finder;
        thread->skeleton = skeleton;
        thread->corner_cycles = corner_cycles;
        thread->edge_cycles = edge_cycles;
        thread->begin = split_points[i];
        thread->end = split_points[i + 1] - 1;
        pthread_create(&thread->thread_id, NULL, WorkerThreadStart, thread);
    }
    for (size_t i = 0; i < thread_count; ++i) {
        pthread_join(worker_threads[i].thread_id, NULL);
    }

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


void* WorkerThreadStart(void* arg) {
    WorkerThread* thread = (WorkerThread*)arg;
    FinderWorkerConstruct(&thread->worker, thread->finder, thread->skeleton);
    FinderWorkerSearch(
        &thread->worker,
        thread->corner_cycles, thread->edge_cycles,
        thread->begin, thread->end
    );
    FinderWorkerDestroy(&thread->worker);
    return NULL;
}


int SolutionCompare(const void* p, const void* q) {
    return ((const Worker*)p)->cancellation - ((const Worker*)q)->cancellation;
}
