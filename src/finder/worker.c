#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "../utils/memory.h"
#include "finder.h"


typedef FinderWorker Worker;

static void search_last_parity(Worker* worker, size_t begin, size_t end);
static void search_last_corner_cycle(Worker* worker, size_t begin, size_t end);
static void search_last_edge_cycle(Worker* worker, size_t begin, size_t end);

static void try_insertion(
    Worker* worker,
    size_t insert_place,
    const Cube* state,
    bool swapped,
    bool parity, int corner_cycles, int edge_cycles
);
static void try_last_insertion(Worker* worker, size_t insert_place, int index);

static void push_insertion(Worker* worker, const Formula* insert_result);
static void pop_insertion(Worker* worker);

static void solution_found(
    Worker* worker,
    size_t insert_place,
    const Algorithm* algorithm
);
static void update_fewest_moves(Worker* worker, size_t moves);

static bool bitcount_less_than_2(uint32_t n);

static bool not_searched(
    const Formula* formula,
    size_t index,
    size_t new_begin,
    bool swappable
);
static bool continue_searching(const Worker* worker, const Formula* formula);


void finder_worker_construct(
    Worker* worker,
    Finder* finder,
    const Formula* skeleton
) {
    worker->finder = finder;
    worker->depth = 0;
    worker->solving_step_capacity = 8;
    worker->solving_step = MALLOC(Insertion, worker->solving_step_capacity);
    formula_duplicate(&worker->solving_step[0].skeleton, skeleton);
}

void finder_worker_destroy(Worker* worker) {
    Insertion* begin = worker->solving_step;
    Insertion* end = begin + worker->depth + 1;
    for (Insertion* p = begin; p < end; ++p) {
        formula_destroy(&p->skeleton);
    }
    free(worker->solving_step);
}


void finder_worker_search(
    Worker* worker,
    bool parity, int corner_cycles, int edge_cycles,
    size_t begin, size_t end
) {
    if (parity && corner_cycles == 0 && edge_cycles == 0) {
        search_last_parity(worker, begin, end);
        return;
    } else if (!parity && corner_cycles == 1 && edge_cycles == 0) {
        search_last_corner_cycle(worker, begin, end);
        return;
    } else if (!parity && corner_cycles == 0 && edge_cycles == 1) {
        search_last_edge_cycle(worker, begin, end);
        return;
    }

    const Finder* finder = worker->finder;
    Cube state = identity_cube;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        Formula* skeleton = &worker->solving_step[worker->depth].skeleton;
        if (insert_place == begin) {
            cube_range_twist_formula(
                &state,
                skeleton,
                insert_place, skeleton->length,
                finder->change_corner, finder->change_edge,
                false
            );
            cube_twist_cube(
                &state,
                &finder->scramble_cube,
                finder->change_corner, finder->change_edge
            );
            cube_range_twist_formula(
                &state,
                skeleton,
                0, insert_place,
                finder->change_corner, finder->change_edge,
                false
            );
        } else {
            int move = skeleton->move[insert_place - 1];
            cube_twist_move_before(
                &state,
                inverse_move_table[move],
                finder->change_corner, finder->change_edge
            );
            cube_twist_move(
                &state,
                move,
                finder->change_corner, finder->change_edge
            );
        }
        try_insertion(
            worker,
            insert_place,
            &state,
            false,
            parity, corner_cycles, edge_cycles
        );

        skeleton = &worker->solving_step[worker->depth].skeleton;
        if (formula_swappable(skeleton, insert_place)) {
            formula_swap_adjacent(skeleton, insert_place);
            const int moves[2] = {
                skeleton->move[insert_place - 1],
                skeleton->move[insert_place]
            };
            Cube swapped_state = identity_cube;
            cube_twist_move(
                &swapped_state,
                moves[1],
                finder->change_corner, finder->change_edge
            );
            cube_twist_move(
                &swapped_state,
                inverse_move_table[moves[0]],
                finder->change_corner, finder->change_edge
            );
            cube_twist_cube(
                &swapped_state,
                &state,
                finder->change_corner, finder->change_edge
            );
            cube_twist_move(
                &swapped_state,
                moves[0],
                finder->change_corner, finder->change_edge
            );
            cube_twist_move(
                &swapped_state,
                inverse_move_table[moves[1]],
                finder->change_corner, finder->change_edge
            );
            try_insertion(
                worker,
                insert_place,
                &swapped_state,
                true,
                parity, corner_cycles, edge_cycles
            );
            formula_swap_adjacent(
                &worker->solving_step[worker->depth].skeleton,
                insert_place
            );
        }
    }
}


void search_last_parity(Worker* worker, size_t begin, size_t end) {
    const Finder* finder = worker->finder;
    int index = -1;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        Formula* skeleton = &worker->solving_step[worker->depth].skeleton;
        if (insert_place == begin) {
            Cube state = identity_cube;
            cube_range_twist_formula(
                &state,
                skeleton,
                0, insert_place,
                true, true,
                true
            );
            cube_twist_cube(&state, &finder->inverse_scramble_cube, true, true);
            cube_range_twist_formula(
                &state,
                skeleton,
                insert_place, skeleton->length,
                true, true,
                true
            );
            index = cube_parity_index(&state);
        } else {
            index = cube_parity_next_index(
                index,
                skeleton->move[insert_place - 1]
            );
        }
        try_last_insertion(
            worker,
            insert_place,
            finder->parity_index[index]
        );

        skeleton = &worker->solving_step[worker->depth].skeleton;
        if (formula_swappable(skeleton, insert_place)) {
            formula_swap_adjacent(skeleton, insert_place);
            int swapped_index = cube_parity_next_index(
                cube_parity_next_index(
                    index,
                    skeleton->move[insert_place - 1]
                ),
                inverse_move_table[skeleton->move[insert_place]]
            );
            try_last_insertion(
                worker,
                insert_place,
                finder->parity_index[swapped_index]
            );
            formula_swap_adjacent(
                &worker->solving_step[worker->depth].skeleton,
                insert_place
            );
        }
    }
}

void search_last_corner_cycle(Worker* worker, size_t begin, size_t end) {
    const Finder* finder = worker->finder;
    int index = -1;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        Formula* skeleton = &worker->solving_step[worker->depth].skeleton;
        if (insert_place == begin) {
            Cube state = identity_cube;
            cube_range_twist_formula(
                &state,
                skeleton,
                0, insert_place,
                true, false,
                true
            );
            cube_twist_cube(
                &state,
                &finder->inverse_scramble_cube,
                true, false
            );
            cube_range_twist_formula(
                &state,
                skeleton,
                insert_place, skeleton->length,
                true, false,
                true
            );
            index = cube_corner_cycle_index(&state);
        } else {
            index = cube_corner_next_cycle_index(
                index,
                skeleton->move[insert_place - 1]
            );
        }
        try_last_insertion(
            worker,
            insert_place,
            finder->corner_cycle_index[index]
        );

        skeleton = &worker->solving_step[worker->depth].skeleton;
        if (formula_swappable(skeleton, insert_place)) {
            formula_swap_adjacent(skeleton, insert_place);
            int swapped_index = cube_corner_next_cycle_index(
                cube_corner_next_cycle_index(
                    index,
                    skeleton->move[insert_place - 1]
                ),
                inverse_move_table[skeleton->move[insert_place]]
            );
            try_last_insertion(
                worker,
                insert_place,
                finder->corner_cycle_index[swapped_index]
            );
            formula_swap_adjacent(
                &worker->solving_step[worker->depth].skeleton,
                insert_place
            );
        }
    }
}

void search_last_edge_cycle(Worker* worker, size_t begin, size_t end) {
    const Finder* finder = worker->finder;
    int index = -1;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        Formula* skeleton = &worker->solving_step[worker->depth].skeleton;
        if (insert_place == begin) {
            Cube state = identity_cube;
            cube_range_twist_formula(
                &state,
                skeleton,
                0, insert_place,
                false, true,
                true
            );
            cube_twist_cube(
                &state,
                &finder->inverse_scramble_cube,
                false, true
            );
            cube_range_twist_formula(
                &state,
                skeleton,
                insert_place, skeleton->length,
                false, true,
                true
            );
            index = cube_edge_cycle_index(&state);
        } else {
            index = cube_edge_next_cycle_index(
                index,
                skeleton->move[insert_place - 1]
            );
        }
        try_last_insertion(
            worker,
            insert_place,
            finder->edge_cycle_index[index]
        );

        skeleton = &worker->solving_step[worker->depth].skeleton;
        if (formula_swappable(skeleton, insert_place)) {
            formula_swap_adjacent(skeleton, insert_place);
            int swapped_index = cube_edge_next_cycle_index(
                cube_edge_next_cycle_index(
                    index,
                    skeleton->move[insert_place - 1]
                ),
                inverse_move_table[skeleton->move[insert_place]]
            );
            try_last_insertion(
                worker,
                insert_place,
                finder->edge_cycle_index[swapped_index]
            );
            formula_swap_adjacent(
                &worker->solving_step[worker->depth].skeleton,
                insert_place
            );
        }
    }
}


void try_insertion(
    Worker* worker,
    size_t insert_place,
    const Cube* state,
    bool swapped,
    bool parity, int corner_cycles, int edge_cycles
) {
    const Finder* finder = worker->finder;
    worker->solving_step[worker->depth].insert_place = insert_place;
    uint32_t insert_place_mask[2];
    formula_get_insert_place_mask(
        &worker->solving_step[worker->depth].skeleton,
        insert_place,
        insert_place_mask
    );
    uint32_t mask = cube_mask(state);
    for (size_t i = 0; i < finder->algorithm_count; ++i) {
        const Algorithm* algorithm = finder->algorithm_list[i];
        if (bitcount_less_than_2(mask & algorithm->mask)) {
            continue;
        }
        bool corner_changed = algorithm->mask & 0xff000;
        bool edge_changed = algorithm->mask & 0xfff;
        Cube cube;
        if (!cube_twist_positive(
            &cube,
            state, &algorithm->state,
            corner_changed, edge_changed
        )) {
            continue;
        }
        bool parity_new = parity ^ algorithm->parity;
        int corner_cycles_new =
            corner_changed ? cube_corner_cycles(&cube) : corner_cycles;
        int edge_cycles_new =
            edge_changed ? cube_edge_cycles(&cube) : edge_cycles;
        if (!parity_new && corner_cycles_new == 0 && edge_cycles_new == 0) {
            solution_found(worker, insert_place, algorithm);
        } else if (
            corner_cycles_new + edge_cycles_new + parity_new
            < corner_cycles + edge_cycles + parity
        ) {
            for (size_t j = 0; j < algorithm->size; ++j) {
                Insertion* insertion = &worker->solving_step[worker->depth];
                const Formula* skeleton = &insertion->skeleton;
                insertion->insertion = &algorithm->formula_list[j];
                if (!formula_insert_is_worthy(
                    skeleton,
                    insert_place,
                    insertion->insertion,
                    insert_place_mask,
                    finder->fewest_moves
                )) {
                    continue;
                }
                Formula formula;
                size_t new_begin = formula_insert(
                    skeleton,
                    insert_place,
                    insertion->insertion,
                    &formula
                );
                if (
                    !not_searched(skeleton, insert_place, new_begin, swapped)
                    || !continue_searching(worker, &formula)
                ) {
                    formula_destroy(&formula);
                    continue;
                }
                push_insertion(worker, &formula);
                finder_worker_search(
                    worker,
                    parity_new, corner_cycles_new, edge_cycles_new,
                    new_begin, formula.length
                );
                pop_insertion(worker);
            }
        }
    }
}

void try_last_insertion(Worker* worker, size_t insert_place, int index) {
    if (index == -1) {
        return;
    }
    const Finder* finder = worker->finder;
    const Algorithm* algorithm = finder->algorithm_list[index];
    worker->solving_step[worker->depth].insert_place = insert_place;
    uint32_t insert_place_mask[2];
    formula_get_insert_place_mask(
        &worker->solving_step[worker->depth].skeleton,
        insert_place,
        insert_place_mask
    );
    for (size_t i = 0; i < algorithm->size; ++i) {
        Insertion* insertion = &worker->solving_step[worker->depth];
        insertion->insertion = &algorithm->formula_list[i];
        if (!formula_insert_is_worthy(
            &insertion->skeleton,
            insert_place,
            insertion->insertion,
            insert_place_mask,
            finder->fewest_moves
        )) {
            continue;
        }
        push_insertion(worker, NULL);
        update_fewest_moves(
            worker,
            worker->solving_step[worker->depth].skeleton.length
        );
        pop_insertion(worker);
    }
}


void push_insertion(Worker* worker, const Formula* insert_result) {
    if (++worker->depth == worker->solving_step_capacity) {
        REALLOC(
            worker->solving_step,
            Insertion,
            worker->solving_step_capacity <<= 1
        );
    }
    Formula* formula = &worker->solving_step[worker->depth].skeleton;
    if (insert_result) {
        memcpy(formula, insert_result, sizeof(Formula));
    } else {
        const Insertion* previous = &worker->solving_step[worker->depth - 1];
        formula_insert(
            &previous->skeleton,
            previous->insert_place,
            previous->insertion,
            formula
        );
    }
}

void pop_insertion(Worker* worker) {
    formula_destroy(&worker->solving_step[worker->depth--].skeleton);
}


void solution_found(
    Worker* worker,
    size_t insert_place,
    const Algorithm* algorithm
) {
    const Finder* finder = worker->finder;
    worker->solving_step[worker->depth].insert_place = insert_place;
    uint32_t insert_place_mask[2];
    formula_get_insert_place_mask(
        &worker->solving_step[worker->depth].skeleton,
        insert_place,
        insert_place_mask
    );
    for (size_t i = 0; i < algorithm->size; ++i) {
        Insertion* insertion = &worker->solving_step[worker->depth];
        insertion->insertion = &algorithm->formula_list[i];
        if (!formula_insert_is_worthy(
            &insertion->skeleton,
            insert_place,
            insertion->insertion,
            insert_place_mask,
            finder->fewest_moves
        )) {
            continue;
        }
        push_insertion(worker, NULL);
        update_fewest_moves(
            worker,
            worker->solving_step[worker->depth].skeleton.length
        );
        pop_insertion(worker);
    }
}

void update_fewest_moves(Worker* worker, size_t moves) {
    Finder* finder = worker->finder;
    if (moves > finder->fewest_moves) {
        return;
    }
    pthread_mutex_lock(&finder->mutex);

    size_t depth = worker->depth;
    const Insertion* worker_steps = worker->solving_step;
    if (moves <= finder->fewest_moves) {
        if (moves < finder->fewest_moves) {
            finder->fewest_moves = moves;
            for (size_t i = 0; i < finder->solution_count; ++i) {
                finder_worker_destroy(&finder->solution_list[i]);
            }
            finder->solution_count = 0;
        }

        if (finder->solution_count == finder->solution_capacity) {
            REALLOC(
                finder->solution_list,
                Worker,
                finder->solution_capacity <<= 1
            );
        }
        Worker* answer = &finder->solution_list[finder->solution_count++];
        answer->depth = depth;
        answer->solving_step_capacity = depth + 1;
        answer->solving_step = MALLOC(Insertion, worker->solving_step_capacity);
        Insertion* answer_steps = answer->solving_step;
        for (size_t i = 0; i < depth; ++i) {
            formula_duplicate(
                &answer_steps[i].skeleton,
                &worker_steps[i].skeleton
            );
            answer_steps[i].insert_place = worker_steps[i].insert_place;
            answer_steps[i].insertion = worker_steps[i].insertion;
        }
        formula_duplicate(
            &answer_steps[depth].skeleton,
            &worker_steps[depth].skeleton
        );
    }

    pthread_mutex_unlock(&finder->mutex);
}


bool bitcount_less_than_2(uint32_t n) {
    return (n & (n - 1)) == 0;
}


bool not_searched(
    const Formula* formula,
    size_t insert_place,
    size_t new_begin,
    bool swappable
) {
    if (
        swappable || insert_place < 2
        || formula_swappable(formula, insert_place - 1)
    ) {
        return new_begin >= insert_place;
    } else {
        return new_begin >= insert_place - 1;
    }
}

bool continue_searching(const Worker* worker, const Formula* formula) {
    return formula->length <= worker->finder->fewest_moves;
}
