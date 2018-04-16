#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
    bool parity;
    int corner_cycles;
    int edge_cycles;
    size_t begin;
    size_t end;
};

static FinderSolveStatus finder_solve_main(
    Finder* finder,
    const Formula* skeleton,
    size_t max_threads
);

static void* worker_thread_start(void* arg);

static int solution_compare(const void* p, const void* q);


void finder_construct(
    Finder* finder,
    size_t algorithm_count, Algorithm** algorithm_list,
    const Formula* scramble
) {
    finder->algorithm_count = algorithm_count;
    finder->algorithm_list = algorithm_list;
    memset(finder->parity_index, -1, 7 * 24 * 11 * 24 * sizeof(int));
    memset(finder->corner_cycle_index, -1, 6 * 24 * 24 * sizeof(int));
    memset(finder->edge_cycle_index, -1, 10 * 24 * 24 * sizeof(int));
    finder->change_parity = false;
    finder->change_corner = false;
    finder->change_edge = false;

    for (size_t i = 0; i < algorithm_count; ++i) {
        const Algorithm* algorithm = algorithm_list[i];
        const Cube* state = &algorithm->state;
        bool parity = algorithm->parity;
        int corner_cycles = algorithm->corner_cycles;
        int edge_cycles = algorithm->edge_cycles;
        int corner_changed = algorithm->mask & 0xff000;
        bool edge_changed = algorithm->mask & 0xfff;
        if (parity) {
            finder->change_parity = true;
        }
        if (corner_changed) {
            finder->change_corner = true;
        }
        if (edge_changed) {
            finder->change_edge = true;
        }
        if (parity && corner_cycles == 0 && edge_cycles == 0) {
            finder->parity_index[cube_parity_index(state)] = i;
        }
        if (!parity && corner_cycles == 1 && edge_cycles == 0) {
            finder->corner_cycle_index[cube_corner_cycle_index(state)] = i;
        }
        if (!parity && corner_cycles == 0 && edge_cycles == 1) {
            finder->edge_cycle_index[cube_edge_cycle_index(state)] = i;
        }
    }

    formula_construct(&finder->scramble, NULL);
    formula_duplicate(&finder->scramble, scramble);
    cube_construct(&finder->scramble_cube);
    cube_twist_formula(&finder->scramble_cube, scramble, true, true, false);
    cube_inverse_state(&finder->scramble_cube, &finder->inverse_scramble_cube);

    finder->fewest_moves = SIZE_MAX;
    finder->solution_count = 0;
    finder->solution_capacity = 16;
    finder->solution_list = MALLOC(Worker, finder->solution_capacity);

    pthread_mutex_init(&finder->mutex, NULL);
}

void finder_destroy(Finder* finder) {
    formula_destroy(&finder->scramble);
    Worker* begin = finder->solution_list;
    Worker* end = begin + finder->solution_count;
    for (Worker* p = begin; p < end; ++p) {
        finder_worker_destroy(p);
    }
    free(finder->solution_list);
    pthread_mutex_destroy(&finder->mutex);
}


FinderSolveResult finder_solve(
    Finder* finder,
    const Formula* skeleton,
    size_t max_threads
) {
    FinderSolveResult result;
    struct timespec time_begin;
    struct timespec time_end;
    struct timespec cpu_time_begin;
    struct timespec cpu_time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_begin);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_time_begin);
    result.status = finder_solve_main(finder, skeleton, max_threads);
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_time_end);
    result.duration =
        (time_end.tv_sec - time_begin.tv_sec) * 1000000000ull
        + (time_end.tv_nsec - time_begin.tv_nsec);
    result.cpu_time =
        (cpu_time_end.tv_sec - cpu_time_begin.tv_sec) * 1000000000ull
        + (cpu_time_end.tv_nsec - cpu_time_begin.tv_nsec);
    return result;
}


FinderSolveStatus finder_solve_main(
    Finder* finder,
    const Formula* skeleton,
    size_t max_threads
) {
    Cube cube = identity_cube;
    cube_twist_formula(&cube, &finder->scramble, true, true, false);
    cube_twist_formula(&cube, skeleton, true, true, false);
    int parity = cube_has_parity(&cube);
    int corner_cycles = cube_corner_cycles(&cube);
    int edge_cycles = cube_edge_cycles(&cube);
    if (!parity && corner_cycles == 0 && edge_cycles == 0) {
        return SOLVE_SUCCESS_SOLVED;
    } else if (parity && !finder->change_parity) {
        return SOLVE_FAILURE_PARITY_ALGORITHMS_NEEDED;
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
        thread->parity = parity;
        thread->corner_cycles = corner_cycles;
        thread->edge_cycles = edge_cycles;
        thread->begin = split_points[i];
        thread->end = split_points[i + 1] - 1;
        pthread_create(&thread->thread_id, NULL, worker_thread_start, thread);
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
        solution_compare
    );

    return SOLVE_SUCCESS;
}


void* worker_thread_start(void* arg) {
    WorkerThread* thread = (WorkerThread*)arg;
    finder_worker_construct(&thread->worker, thread->finder, thread->skeleton);
    finder_worker_search(
        &thread->worker,
        thread->parity, thread->corner_cycles, thread->edge_cycles,
        thread->begin, thread->end
    );
    finder_worker_destroy(&thread->worker);
    return NULL;
}


int solution_compare(const void* p, const void* q) {
    return ((const Worker*)p)->cancellation - ((const Worker*)q)->cancellation;
}
