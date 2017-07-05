#include <limits.h>
#include <stdlib.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "finder.h"


typedef FinderWorker Worker;

static void SearchLastCornerCycle(Worker* worker, size_t begin, size_t end);
static void SearchLastEdgeCycle(Worker* worker, size_t begin, size_t end);

static void TryInsertion(
    Worker* worker,
    size_t insert_place,
    const Cube* state,
    bool swapped,
    int corner_cycles, int edge_cycles
);
static void TryLastInsertion(Worker* worker, size_t insert_place, int index);

static void PushInsertion(Worker* worker, const Formula* inserted);
static void PopInsertion(Worker* worker);

static void SolutionFound(
    Worker* worker,
    size_t insert_place,
    const Algorithm* algorithm
);
static void UpdateFewestMoves(Worker* worker);
static void SetFewestMoves(Worker* worker, size_t moves);

static bool BitCountLessThan2(unsigned n);

static bool NotSearched(
    const Formula* formula,
    size_t index,
    size_t new_begin,
    bool swappable
);
static bool ContinueSearching(const Worker* worker, const Formula* formula);


void FinderWorkerConstruct(
    Worker* worker,
    Finder* finder,
    const Formula* skeleton
) {
    worker->finder = finder;
    worker->depth = 0;
    worker->solving_step_capacity = 8;
    worker->solving_step = (Insertion*)malloc(
        worker->solving_step_capacity * sizeof(Insertion)
    );
    FormulaDuplicate(
        &worker->solving_step[0].skeleton,
        skeleton
    );
}

void FinderWorkerDestroy(Worker* worker) {
    for (size_t i = 0; i <= worker->depth; ++i) {
        FormulaDestroy(&worker->solving_step[i].skeleton);
    }
    free(worker->solving_step);
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
    Formula* skeleton = &insertion->skeleton;
    Cube state;
    CubeConstruct(&state);
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            CubeRangeTwistFormula(
                &state,
                skeleton,
                insert_place, skeleton->length,
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
                skeleton,
                0, insert_place,
                finder->change_corner, finder->change_edge,
                false
            );
        } else {
            int move = skeleton->move[insert_place - 1];
            CubeTwistBefore(
                &state,
                inverse_move_table[move],
                finder->change_corner, finder->change_edge
            );
            CubeTwist(&state, move, finder->change_corner, finder->change_edge);
        }
        TryInsertion(
            worker,
            insert_place,
            &state,
            false,
            corner_cycles, edge_cycles
        );

        if (FormulaSwappable(skeleton, insert_place)) {
            FormulaSwapAdjacent(skeleton, insert_place);
            const int moves[2] = {
                skeleton->move[insert_place - 1],
                skeleton->move[insert_place]
            };
            Cube swapped_state;
            CubeConstruct(&swapped_state);
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
                true,
                corner_cycles, edge_cycles
            );
            FormulaSwapAdjacent(skeleton, insert_place);
        }
    }
}


void SearchLastCornerCycle(Worker* worker, size_t begin, size_t end) {
    const Finder* finder = worker->finder;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        Insertion* insertion = &worker->solving_step[worker->depth];
        Formula* skeleton = &insertion->skeleton;
        int index;
        if (insert_place == begin) {
            Cube state;
            CubeConstruct(&state);
            CubeRangeTwistFormula(
                &state,
                skeleton,
                0, insert_place,
                true, false,
                true
            );
            CubeTwistCube(&state, &finder->inverse_scramble_cube, true, false);
            CubeRangeTwistFormula(
                &state,
                skeleton,
                insert_place, skeleton->length,
                true, false,
                true
            );
            index = CubeCorner3CycleIndex(&state);
        } else {
            index = CubeCornerNext3CycleIndex(
                index,
                skeleton->move[insert_place - 1]
            );
        }
        TryLastInsertion(
            worker,
            insert_place,
            finder->corner_cycle_index[index]
        );

        if (FormulaSwappable(skeleton, insert_place)) {
            FormulaSwapAdjacent(skeleton, insert_place);
            int swapped_index = CubeCornerNext3CycleIndex(
                index,
                skeleton->move[insert_place - 1]
            );
            swapped_index = CubeCornerNext3CycleIndex(
                swapped_index,
                inverse_move_table[skeleton->move[insert_place]]
            );
            TryLastInsertion(
                worker,
                insert_place,
                finder->corner_cycle_index[swapped_index]
            );
            FormulaSwapAdjacent(skeleton, insert_place);
        }
    }
}

void SearchLastEdgeCycle(Worker* worker, size_t begin, size_t end) {
    const Finder* finder = worker->finder;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        Insertion* insertion = &worker->solving_step[worker->depth];
        Formula* skeleton = &insertion->skeleton;
        int index;
        if (insert_place == begin) {
            Cube state;
            CubeConstruct(&state);
            CubeRangeTwistFormula(
                &state,
                skeleton,
                0, insert_place,
                false, true,
                true
            );
            CubeTwistCube(&state, &finder->inverse_scramble_cube, false, true);
            CubeRangeTwistFormula(
                &state,
                skeleton,
                insert_place, skeleton->length,
                false, true,
                true
            );
            index = CubeEdge3CycleIndex(&state);
        } else {
            index = CubeEdgeNext3CycleIndex(
                index,
                skeleton->move[insert_place - 1]
            );
        }
        TryLastInsertion(worker, insert_place, finder->edge_cycle_index[index]);

        if (FormulaSwappable(skeleton, insert_place)) {
            FormulaSwapAdjacent(skeleton, insert_place);
            int swapped_index = CubeEdgeNext3CycleIndex(
                index,
                skeleton->move[insert_place - 1]
            );
            swapped_index = CubeEdgeNext3CycleIndex(
                swapped_index,
                inverse_move_table[skeleton->move[insert_place]]
            );
            TryLastInsertion(
                worker,
                insert_place,
                finder->edge_cycle_index[swapped_index]
            );
            FormulaSwapAdjacent(skeleton, insert_place);
        }
    }
}


void TryInsertion(
    Worker* worker,
    size_t insert_place,
    const Cube* state,
    bool swapped,
    int corner_cycles, int edge_cycles
) {
    const Finder* finder = worker->finder;
    Insertion* insertion = &worker->solving_step[worker->depth];
    const Formula* skeleton = &insertion->skeleton;
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
                insertion->insertion = &algorithm->formula_list[j];
                if (!FormulaInsertIsWorthy(
                    skeleton,
                    insert_place,
                    insertion->insertion
                )) {
                    continue;
                }
                Formula formula;
                size_t new_begin = FormulaInsert(
                    skeleton,
                    insert_place,
                    insertion->insertion,
                    &formula
                );
                if (!NotSearched(skeleton, insert_place, new_begin, swapped)) {
                    continue;
                } else if (!ContinueSearching(worker, &formula)) {
                    continue;
                }
                insertion->insert_place = insert_place;
                PushInsertion(worker, &formula);
                FinderWorkerSearch(
                    worker,
                    corner_cycles_new, edge_cycles_new,
                    new_begin, formula.length
                );
                PopInsertion(worker);
            }
        }
    }
}

void TryLastInsertion(Worker* worker, size_t insert_place, int index) {
    if (index == -1) {
        return;
    }
    const Finder* finder = worker->finder;
    const Algorithm* algorithm = finder->algorithm_list[index];
    Insertion* insertion = &worker->solving_step[worker->depth];
    const Formula* skeleton = &insertion->skeleton;
    insertion->insert_place = insert_place;
    for (size_t i = 0; i < algorithm->size; ++i) {
        insertion->insertion = &algorithm->formula_list[i];
        size_t new_length = skeleton->length + insertion->insertion->length;
        ptrdiff_t moves_to_cancel = new_length - finder->fewest_moves;
        if (finder->fewest_moves == ULONG_MAX || moves_to_cancel <= 0) {
            SetFewestMoves(worker, new_length);
            if (!FormulaInsertIsWorthy(
                skeleton,
                insert_place,
                insertion->insertion
            )) {
                continue;
            }
        } else {
            if (!FormulaInsertFinalIsWorthy(
                skeleton,
                insert_place,
                insertion->insertion,
                moves_to_cancel
            )) {
                continue;
            }
        }
        PushInsertion(worker, NULL);
        UpdateFewestMoves(worker);
        PopInsertion(worker);
    }
}


void PushInsertion(Worker* worker, const Formula* inserted) {
    if (++worker->depth == worker->solving_step_capacity) {
        worker->solving_step = (Insertion*)realloc(
            worker->solving_step,
            (worker->solving_step_capacity <<= 1) * sizeof(Insertion)
        );
    }
    Formula* formula = &worker->solving_step[worker->depth].skeleton;
    if (inserted) {
        formula->length = inserted->length;
        formula->capacity = inserted->capacity;
        formula->move = inserted->move;
    } else {
        const Insertion* previous = &worker->solving_step[worker->depth - 1];
        FormulaInsert(
            &previous->skeleton,
            previous->insert_place,
            previous->insertion,
            formula
        );
    }
}

void PopInsertion(Worker* worker) {
    FormulaDestroy(&worker->solving_step[worker->depth--].skeleton);
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
    size_t moves = worker_steps[depth].skeleton.length;
    if (moves > finder->fewest_moves) {
        return;
    }
    SetFewestMoves(worker, moves);

    if (finder->solution_count == finder->solution_capacity) {
        finder->solution_list = (Worker*)realloc(
            finder->solution_list,
            (finder->solution_capacity <<= 1) * sizeof(Worker)
        );
    }
    Worker* answer = &finder->solution_list[finder->solution_count++];
    answer->depth = depth;
    answer->solving_step_capacity = depth + 1;
    answer->solving_step = (Insertion*)malloc(
        worker->solving_step_capacity * sizeof(Insertion)
    );
    Insertion* answer_steps = answer->solving_step;
    for (size_t i = 0; i < depth; ++i) {
        FormulaDuplicate(&answer_steps[i].skeleton, &worker_steps[i].skeleton);
        answer_steps[i].insert_place = worker_steps[i].insert_place;
        answer_steps[i].insertion = worker_steps[i].insertion;
    }
    FormulaDuplicate(
        &answer_steps[depth].skeleton,
        &worker_steps[depth].skeleton
    );
}

void SetFewestMoves(Worker* worker, size_t moves) {
    Finder* finder = worker->finder;
    if (moves < finder->fewest_moves) {
        finder->fewest_moves = moves;
        for (size_t i = 0; i < finder->solution_count; ++i) {
            FinderWorkerDestroy(&finder->solution_list[i]);
        }
        finder->solution_count = 0;
    }
}


bool BitCountLessThan2(unsigned n) {
    return (n & (n - 1)) == 0;
}


bool NotSearched(
    const Formula* formula,
    size_t insert_place,
    size_t new_begin,
    bool swappable
) {
    if (
        swappable || insert_place < 2
        || FormulaSwappable(formula, insert_place - 1)
    ) {
        return new_begin >= insert_place;
    } else {
        return new_begin >= insert_place - 1;
    }
}

bool ContinueSearching(const Worker* worker, const Formula* formula) {
    return formula->length <= worker->finder->fewest_moves;
}
