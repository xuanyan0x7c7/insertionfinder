#include <cstdint>
#include <utility>
#include <algorithm.hpp>
#include <case.hpp>
#include <cube.hpp>
#include <finder/brute-force.hpp>
#include "utils.hpp"
using namespace std;
using namespace InsertionFinder;
using namespace Details;


namespace {
    bool not_searched(
        const Algorithm& algorithm, size_t insert_place,
        size_t new_begin,
        bool swapped
    ) {
        if (swapped || insert_place < 2 || algorithm.swappable(insert_place - 1)) {
            return new_begin >= insert_place;
        } else {
            return new_begin >= insert_place - 1;
        }
    }
};


BruteForceFinder::Worker::Worker(BruteForceFinder& finder):
    finder(finder),
    solving_step({{finder.skeleton, 0, nullptr}}) {}


void BruteForceFinder::Worker::search(
    const BruteForceFinder::CycleStatus& cycle_status,
    size_t begin, size_t end
) {
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    int placement = cycle_status.placement;
    if (parity && corner_cycles == 0 && edge_cycles == 0 && placement == 0) {
        this->search_last_parity(begin, end);
        return;
    } else if (!parity && corner_cycles == 1 && edge_cycles == 0 && placement == 0) {
        this->search_last_corner_cycle(begin, end);
        return;
    } else if (!parity && corner_cycles == 0 && edge_cycles == 1 && placement == 0) {
        this->search_last_edge_cycle(begin, end);
        return;
    } else if (
        !parity && corner_cycles == 0 && edge_cycles == 0
        && !Cube::placement_parity(placement)
    ) {
        this->search_last_placement(placement, begin, end);
        return;
    }

    Algorithm skeleton = this->solving_step.back().skeleton;
    const int* transform = rotation_permutation[placement];
    byte twist_flag;
    if (this->finder.change_corner) {
        twist_flag |= CubeTwist::corners;
    }
    if (this->finder.change_edge) {
        twist_flag |= CubeTwist::edges;
    }

    Cube state;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            state.rotate(placement);
            for (size_t k = insert_place; k < skeleton.length(); ++k) {
                state.twist(
                    transform_twist(transform, skeleton[insert_place]),
                    twist_flag
                );
            }
            state.twist(this->finder.scramble_cube, twist_flag);
            state.twist(skeleton, 0, insert_place, twist_flag);
        } else {
            int twist = skeleton[insert_place - 1];
            state.twist_before(
                transform_twist(transform, Algorithm::inverse_twist[twist]),
                twist_flag
            );
            state.twist(twist, twist_flag);
        }
        this->try_insertion(insert_place, state, cycle_status);

        if (skeleton.swappable(insert_place)) {
            skeleton.swap_adjacent(insert_place);
            const int twists[2] = {
                skeleton[insert_place - 1],
                skeleton[insert_place]
            };
            Cube swapped_state;
            swapped_state.rotate(placement);
            swapped_state.twist(
                transform_twist(transform, twists[1]),
                twist_flag
            );
            swapped_state.twist(
                transform_twist(transform, Algorithm::inverse_twist[twists[0]]),
                twist_flag
            );
            swapped_state.twist(state, twist_flag);
            swapped_state.twist(twists[0], twist_flag);
            swapped_state.twist(Algorithm::inverse_twist[twists[1]], twist_flag);
            this->try_insertion(insert_place, swapped_state, cycle_status, true);
            skeleton.swap_adjacent(insert_place);
        }
    }
}

void BruteForceFinder::Worker::search_last_parity(size_t begin, size_t end) {
    Algorithm skeleton = this->solving_step.back().skeleton;
    const byte twist_flag = CubeTwist::corners | CubeTwist::edges | CubeTwist::reversed;
    const auto& parity_index = this->finder.parity_index;

    int index = -1;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            Cube state;
            state.twist(skeleton, 0, insert_place, twist_flag);
            state.twist(this->finder.inverse_scramble_cube, twist_flag);
            state.twist(skeleton, insert_place, skeleton.length(), twist_flag);
            index = state.parity_index();
        } else {
            index = Cube::next_parity_index(index, skeleton[insert_place - 1]);
        }
        this->try_last_insertion(insert_place, parity_index[index]);

        if (skeleton.swappable(insert_place)) {
            skeleton.swap_adjacent(insert_place);
            int swapped_index = Cube::next_parity_index(
                Cube::next_parity_index(index, skeleton[insert_place - 1]),
                Algorithm::inverse_twist[skeleton[insert_place]]
            );
            this->try_last_insertion(insert_place, parity_index[swapped_index], true);
            skeleton.swap_adjacent(insert_place);
        }
    }
}

void BruteForceFinder::Worker::search_last_corner_cycle(size_t begin, size_t end) {
    Algorithm skeleton = this->solving_step.back().skeleton;
    const byte twist_flag = CubeTwist::corners | CubeTwist::reversed;
    const auto& corner_cycle_index = this->finder.corner_cycle_index;

    int index = -1;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            Cube state;
            state.twist(skeleton, 0, insert_place, twist_flag);
            state.twist(this->finder.inverse_scramble_cube, twist_flag);
            state.twist(skeleton, insert_place, skeleton.length(), twist_flag);
            index = state.corner_cycle_index();
        } else {
            index = Cube::next_corner_cycle_index(index, skeleton[insert_place - 1]);
        }
        this->try_last_insertion(insert_place, corner_cycle_index[index]);

        if (skeleton.swappable(insert_place)) {
            skeleton.swap_adjacent(insert_place);
            int swapped_index = Cube::next_corner_cycle_index(
                Cube::next_corner_cycle_index(index, skeleton[insert_place - 1]),
                Algorithm::inverse_twist[skeleton[insert_place]]
            );
            this->try_last_insertion(insert_place, corner_cycle_index[swapped_index], true);
            skeleton.swap_adjacent(insert_place);
        }
    }
}

void BruteForceFinder::Worker::search_last_edge_cycle(size_t begin, size_t end) {
    Algorithm skeleton = this->solving_step.back().skeleton;
    const byte twist_flag = CubeTwist::edges | CubeTwist::reversed;
    const auto& edge_cycle_index = this->finder.edge_cycle_index;

    int index = -1;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            Cube state;
            state.twist(skeleton, 0, insert_place, twist_flag);
            state.twist(this->finder.inverse_scramble_cube, twist_flag);
            state.twist(skeleton, insert_place, skeleton.length(), twist_flag);
            index = state.edge_cycle_index();
        } else {
            index = Cube::next_edge_cycle_index(index, skeleton[insert_place - 1]);
        }
        this->try_last_insertion(insert_place, edge_cycle_index[index]);

        if (skeleton.swappable(insert_place)) {
            skeleton.swap_adjacent(insert_place);
            int swapped_index = Cube::next_edge_cycle_index(
                Cube::next_edge_cycle_index(index, skeleton[insert_place - 1]),
                Algorithm::inverse_twist[skeleton[insert_place]]
            );
            this->try_last_insertion(insert_place, edge_cycle_index[swapped_index], true);
            skeleton.swap_adjacent(insert_place);
        }
    }
}

void BruteForceFinder::Worker::search_last_placement(
    int placement,
    size_t begin, size_t end
) {
    Algorithm skeleton = this->solving_step.back().skeleton;
    int case_index = this->finder.center_index[inverse_center[placement]];
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        this->try_last_insertion(insert_place, case_index);
        if (skeleton.swappable(insert_place)) {
            this->try_last_insertion(insert_place, case_index, true);
        }
    }
}


void BruteForceFinder::Worker::try_insertion(
    size_t insert_place,
    const Cube& state,
    const CycleStatus& cycle_status,
    bool swapped
) {
    if (swapped) {
        this->solving_step.back().skeleton.swap_adjacent(insert_place);
    }
    this->solving_step.back().insert_place = insert_place;
    uint32_t mask = state.mask();
    auto insert_place_mask = this->solving_step.back().skeleton
        .get_insert_place_mask(insert_place);
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    int placement = cycle_status.placement;

    for (const Case& _case: this->finder.cases) {
        if (bitcount_less_than_2(mask & _case.mask())) {
            continue;
        }
        bool corner_changed = _case.mask() & 0xff;
        bool edge_changed = _case.mask() & 0xfff00;
        bool center_changed = _case.mask() & 0x300000;
        auto cube = state.twist_effectively(
            _case.state(),
            (corner_changed ? CubeTwist::corners : byte{0})
                | (edge_changed ? CubeTwist::edges : byte{0})
                | (center_changed ? CubeTwist::centers : byte{0})
        );
        if (!cube) {
            continue;
        }
        bool new_parity = parity ^ _case.has_parity();
        int new_corner_cycles = corner_changed ? cube->corner_cycles() : corner_cycles;
        int new_edge_cycles = edge_changed ? cube->edge_cycles() : edge_cycles;
        int new_placement = Cube::placement_twist(placement, _case.rotation());
        if (
            !new_parity
            && new_corner_cycles == 0 && new_edge_cycles == 0
            && new_placement == 0
        ) {
            this->solution_found(insert_place, _case);
        } else if (
            new_parity + new_corner_cycles + new_edge_cycles + center_cycles[new_placement]
            < parity + corner_cycles + edge_cycles + center_cycles[placement]
        ) {
            for (const Algorithm& algorithm: _case.algorithm_list()) {
                Insertion& insertion = this->solving_step.back();
                insertion.insertion = &algorithm;
                if (!insertion.skeleton.is_worthy_insertion(
                    algorithm, insert_place,
                    insert_place_mask,
                    this->finder.fewest_moves
                )) {
                    continue;
                }
                auto [new_skeleton, new_begin] = insertion.skeleton.insert(algorithm, insert_place);
                if (
                    not_searched(insertion.skeleton, insert_place, new_begin, swapped)
                    && this->continue_searching(algorithm)
                ) {
                    this->solving_step.push_back({move(new_skeleton), 0, nullptr});
                    this->search(
                        {new_parity, new_corner_cycles, new_edge_cycles, new_placement},
                        new_begin, new_skeleton.length()
                    );
                    this->solving_step.pop_back();
                }
            }
        }
    }
}


void BruteForceFinder::Worker::solution_found(size_t insert_place, const Case& _case) {
    this->solving_step.back().insert_place = insert_place;
    auto insert_place_mask = this->solving_step.back().skeleton
        .get_insert_place_mask(insert_place);
    for (const Algorithm& algorithm: _case.algorithm_list()) {
        Insertion& insertion = this->solving_step.back();
        insertion.insertion = &algorithm;
        if (!insertion.skeleton.is_worthy_insertion(
            algorithm, insert_place,
            insert_place_mask,
            this->finder.fewest_moves
        )) {
            continue;
        }
        this->solving_step.push_back({
            insertion.skeleton.insert(algorithm, insert_place).first,
            0, nullptr
        });
        this->update_fewest_moves();
        this->solving_step.pop_back();
    }
}


void BruteForceFinder::Worker::update_fewest_moves() {
    BruteForceFinder& finder = this->finder;
    size_t twists = this->solving_step.back().skeleton.length();
    if (twists > finder.fewest_moves) {
        return;
    }
    lock_guard<mutex> lock(finder.fewest_moves_mutex);
    if (twists > finder.fewest_moves) {
        return;
    }
    if (twists < finder.fewest_moves) {
        finder.fewest_moves = twists;
    }
    finder.solutions.push_back({this->solving_step});
}
