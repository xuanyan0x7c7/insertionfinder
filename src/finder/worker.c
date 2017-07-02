#include <stdlib.h>
#include "../algorithm.h"
#include "../cube.h"
#include "../formula.h"
#include "finder.h"


typedef FinderWorker Worker;

static void SearchLastCornerCycle(Worker* worker, size_t begin, size_t end);
static void SearchLastEdgeCycle(Worker* worker, size_t begin, size_t end);

static void TryInsertion(
    Worker* worker,
    size_t insert_place,
    const Cube* state,
    int corner_cycles, int edge_cycles
);

static void PushInsertion(Worker* worker, const Formula* inserted);
static void PopInsertion(Worker* worker);

static void SolutionFound(
    Worker* worker,
    size_t insert_place,
    const Algorithm* algorithm
);
static void UpdateFewestMoves(Worker* worker);

static bool BitCountLessThan2(unsigned n);

static bool NotSearched(
    const Formula* formula,
    size_t index,
    size_t new_begin,
    bool swappable
);
static bool ContinueSearching(const Worker* worker, int cycles);


FinderWorker* FinderWorkerConstruct(Worker* worker, Finder* finder) {
    if (!worker) {
        worker = (Worker*)malloc(sizeof(Worker));
    }
    worker->finder = finder;
    worker->depth = 0;
    worker->solving_step_capacity = 8;
    worker->solving_step = (Insertion*)malloc(
        worker->solving_step_capacity * sizeof(Insertion)
    );
    InsertionConstruct(worker->solving_step, &finder->scramble);
    return worker;
}

void FinderWorkerDestroy(Worker* worker) {
    for (size_t i = 0; i <= worker->depth; ++i) {
        InsertionDestroy(&worker->solving_step[i]);
    }
    free(worker->solving_step);
    worker->solving_step = NULL;
}


void FinderWorkerSearch(
    Worker* worker,
    int corner_cycles, int edge_cycles,
    size_t begin, size_t end
) {
    if (corner_cycles == 1 && edge_cycles == 0) {
        SearchLastCornerCycle(worker, begin, end);
        return;
    } else if (corner_cycles == 0 && edge_cycles == 1) {
        SearchLastEdgeCycle(worker, begin, end);
        return;
    }

    const Finder* finder = worker->finder;
    Insertion* insertion = &worker->solving_step[worker->depth];
    Formula* partial_solution = &insertion->partial_solution;
    Cube state;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            CubeRangeTwistFormula(
                &state,
                partial_solution,
                insert_place, partial_solution->length,
                finder->change_corner, finder->change_edge,
                false
            );
            CubeTwistCube(
                &state,
                &finder->scramble_cube,
                finder->change_corner, finder->change_edge
            );
            CubeRangeTwistFormula(
                &state,
                partial_solution,
                0, insert_place,
                finder->change_corner, finder->change_edge,
                false
            );
        } else {
            int move = partial_solution->move[insert_place - 1];
            CubeTwistBefore(
                &state,
                inverse_move_table[move],
                finder->change_corner, finder->change_edge
            );
            CubeTwist(&state, move, finder->change_corner, finder->change_edge);
        }
        TryInsertion(worker, insert_place, &state, corner_cycles, edge_cycles);

        if (
            insert_place > 0
            && insert_place < partial_solution->length
            && FormulaSwappable(partial_solution, insert_place)
        ) {
            FormulaSwapAdjacent(partial_solution, insert_place);
            Cube swapped_state;
            const int moves[2] = {
                partial_solution->move[insert_place - 1],
                partial_solution->move[insert_place]
            };
            CubeConstruct(&swapped_state, NULL);
            CubeTwist(
                &swapped_state,
                moves[1],
                finder->change_corner, finder->change_edge
            );
            CubeTwist(
                &swapped_state,
                inverse_move_table[moves[0]],
                finder->change_corner, finder->change_edge
            );
            CubeTwistCube(
                &swapped_state,
                &state,
                finder->change_corner, finder->change_edge
            );
            CubeTwist(
                &swapped_state,
                moves[0],
                finder->change_corner, finder->change_edge
            );
            CubeTwist(
                &swapped_state,
                inverse_move_table[moves[1]],
                finder->change_corner, finder->change_edge
            );
            TryInsertion(
                worker,
                insert_place,
                &swapped_state,
                corner_cycles, edge_cycles
            );
            FormulaSwapAdjacent(partial_solution, insert_place);
        }
    }
}


void SearchLastCornerCycle(Worker* worker, size_t begin, size_t end) {
    const Finder* finder = worker->finder;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        Insertion* insertion = &worker->solving_step[worker->depth];
        int index;
        if (insert_place == begin) {
            Cube state;
            CubeConstruct(&state, NULL);
            CubeRangeTwistFormula(
                &state,
                &insertion->partial_solution,
                0, insert_place,
                true, false,
                true
            );
            CubeTwistCube(&state, &finder->inverse_scramble_cube, true, false);
            CubeRangeTwistFormula(
                &state,
                &insertion->partial_solution,
                insert_place, insertion->partial_solution.length,
                true, false,
                true
            );
            index = CubeCorner3CycleIndex(&state);
        } else {
            index = CubeCornerNext3CycleIndex(
                index,
                insertion->partial_solution.move[insert_place - 1]
            );
        }
        int x = finder->corner_cycle_index[index];
        if (x != -1) {
            const Algorithm* algorithm = finder->algorithm_list[x];
            insertion->insert_place = insert_place;
            for (size_t i = 0; i < algorithm->size; ++i) {
                insertion->insertion = &algorithm->formula_list[i];
                PushInsertion(worker, NULL);
                UpdateFewestMoves(worker);
                PopInsertion(worker);
            }
        }
    }
}

void SearchLastEdgeCycle(Worker* worker, size_t begin, size_t end) {
    const Finder* finder = worker->finder;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        Insertion* insertion = &worker->solving_step[worker->depth];
        int index;
        if (insert_place == begin) {
            Cube state;
            CubeConstruct(&state, NULL);
            CubeRangeTwistFormula(
                &state,
                &insertion->partial_solution,
                0, insert_place,
                false, true,
                true
            );
            CubeTwistCube(&state, &finder->inverse_scramble_cube, false, true);
            CubeRangeTwistFormula(
                &state,
                &insertion->partial_solution,
                insert_place, insertion->partial_solution.length,
                false, true,
                true
            );
            index = CubeEdge3CycleIndex(&state);
        } else {
            index = CubeEdgeNext3CycleIndex(
                index,
                insertion->partial_solution.move[insert_place - 1]
            );
        }
        int x = finder->edge_cycle_index[index];
        if (x != -1) {
            const Algorithm* algorithm = finder->algorithm_list[x];
            insertion->insert_place = insert_place;
            for (size_t i = 0; i < algorithm->size; ++i) {
                insertion->insertion = &algorithm->formula_list[i];
                PushInsertion(worker, NULL);
                UpdateFewestMoves(worker);
                PopInsertion(worker);
            }
        }
    }
}


void TryInsertion(
    Worker* worker,
    size_t insert_place,
    const Cube* state,
    int corner_cycles, int edge_cycles
) {
    const Finder* finder = worker->finder;
    Insertion* insertion = &worker->solving_step[worker->depth];
    const Formula* partial_solution = &insertion->partial_solution;
    unsigned mask = CubeMask(state);
    for (size_t i = 0; i < finder->algorithm_count; ++i) {
        const Algorithm* algorithm = finder->algorithm_list[i];
        if (BitCountLessThan2(mask & algorithm->mask)) {
            continue;
        }
        bool corner_changed = algorithm->mask & 0xff000;
        bool edge_changed = algorithm->mask & 0xfff;
        Cube cube;
        if (!CubeTwistPositive(
            &cube,
            state, &algorithm->state,
            corner_changed, edge_changed
        )) {
            continue;
        }
        int corner_cycles_new =
            corner_changed ? CubeCornerCycles(&cube) : corner_cycles;
        int edge_cycles_new =
            edge_changed ? CubeEdgeCycles(&cube) : edge_cycles;
        if (corner_cycles_new == 0 && edge_cycles_new == 0) {
            SolutionFound(worker, insert_place, algorithm);
        } else if (
            corner_cycles_new + edge_cycles_new
            < corner_cycles + edge_cycles
        ) {
            for (size_t j = 0; j < algorithm->size; ++j) {
                Formula formula;
                size_t new_begin = FormulaInsert(
                    partial_solution,
                    insert_place,
                    &algorithm->formula_list[j],
                    &formula
                );
                if (NotSearched(
                    partial_solution,
                    insert_place,
                    new_begin,
                    true
                ) && ContinueSearching(
                    worker,
                    corner_cycles_new + edge_cycles_new
                )) {
                    insertion->insertion = &algorithm->formula_list[j];
                    PushInsertion(worker, &formula);
                    FinderWorkerSearch(
                        worker,
                        corner_cycles_new, edge_cycles_new,
                        new_begin,
                        formula.length
                    );
                    PopInsertion(worker);
                }
            }
        }
    }
}


void PushInsertion(Worker* worker, const Formula* inserted) {
    if (++worker->depth == worker->solving_step_capacity) {
        worker->solving_step = (Insertion*)realloc(
            worker->solving_step,
            (worker->solving_step_capacity <<= 1) * sizeof(Insertion)
        );
    }
    if (inserted) {
        InsertionConstruct(&worker->solving_step[worker->depth], inserted);
    } else {
        Formula formula;
        FormulaInsert(
            &worker->solving_step[worker->depth - 1].partial_solution,
            worker->solving_step[worker->depth - 1].insert_place,
            worker->solving_step[worker->depth - 1].insertion,
            &formula
        );
        InsertionConstruct(&worker->solving_step[worker->depth], &formula);
    }
}

void PopInsertion(Worker* worker) {
    FormulaDestroy(&worker->solving_step[worker->depth--].partial_solution);
}


void SolutionFound(
    Worker* worker,
    size_t insert_place,
    const Algorithm* algorithm
) {
    Insertion* insertion = &worker->solving_step[worker->depth];
    insertion->insert_place = insert_place;
    for (size_t i = 0; i < algorithm->size; ++i) {
        insertion->insertion = &algorithm->formula_list[i];
        PushInsertion(worker, NULL);
        UpdateFewestMoves(worker);
        PopInsertion(worker);
    }
}

void UpdateFewestMoves(Worker* worker) {
    Finder* finder = worker->finder;
    size_t depth = worker->depth;
    const Insertion* worker_steps = worker->solving_step;
    size_t moves = worker_steps[depth].partial_solution.length;
    if (moves > finder->fewest_moves) {
        return;
    }
    if (moves < finder->fewest_moves) {
        finder->fewest_moves = moves;
        for (size_t i = 0; i < finder->solution_count; ++i) {
            FinderWorkerDestroy(&finder->solution_list[i]);
        }
        finder->solution_count = 0;
    }
    size_t cancelled_moves = worker_steps[0].partial_solution.length;
    for (size_t i = 0; i < depth; ++i) {
        cancelled_moves += worker_steps[i].insertion->length;
    }
    cancelled_moves -= worker_steps[depth].partial_solution.length;

    if (finder->solution_count == finder->solution_capacity) {
        finder->solution_list = (Worker*)realloc(
            finder->solution_list,
            (finder->solution_capacity <<= 1) * sizeof(Worker)
        );
    }
    Worker* answer = &finder->solution_list[finder->solution_count++];
    Insertion* answer_steps = answer->solving_step;
    answer->depth = depth;
    answer->solving_step_capacity = depth + 1;
    answer->solving_step = (Insertion*)malloc(
        worker->solving_step_capacity * sizeof(Insertion)
    );
    for (size_t i = 0; i < depth; ++i) {
        FormulaDuplicate(
            &answer_steps[i].partial_solution,
            &worker_steps[i].partial_solution
        );
        answer_steps[i].insert_place = worker_steps[i].insert_place;
        answer_steps[i].insertion = worker_steps[i].insertion;
    }
    InsertionConstruct(
        &answer_steps[depth],
        &worker_steps[depth].partial_solution
    );
}


bool BitCountLessThan2(unsigned n) {
    return (n & (n - 1)) == 0;
}


bool NotSearched(
    const Formula* formula,
    size_t index,
    size_t new_begin,
    bool swappable
) {
    if (swappable || index < 2 || FormulaSwappable(formula, index - 1)) {
        return new_begin >= index;
    } else {
        return new_begin >= index - 1;
    }
}

bool ContinueSearching(const Worker* worker, int cycles) {
    return worker->solving_step[worker->depth].partial_solution.length
        <= worker->finder->fewest_moves;
}
