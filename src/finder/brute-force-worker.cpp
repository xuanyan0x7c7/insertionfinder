#include <cstdint>
#include <iostream>
#include <utility>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/brute-force.hpp>
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


void BruteForceFinder::Worker::search(const CycleStatus& cycle_status, size_t begin, size_t end) {
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    int placement = cycle_status.placement;
    if (!parity && corner_cycles == 1 && edge_cycles == 0 && placement == 0) {
        this->search_last_corner_cycle(begin, end);
        return;
    } else if (!parity && corner_cycles == 0 && edge_cycles == 1 && placement == 0) {
        this->search_last_edge_cycle(begin, end);
        return;
    } else if (
        !parity && corner_cycles == 0 && edge_cycles == 0
        && Cube::center_cycles[placement] == 1
    ) {
        this->search_last_placement(placement, begin, end);
        return;
    }

    const Algorithm skeleton = this->solving_step.back().skeleton;
    byte twist_flag{0};
    if (this->finder.change_corner) {
        twist_flag |= CubeTwist::corners;
    }
    if (this->finder.change_edge) {
        twist_flag |= CubeTwist::edges;
    }
    if (this->finder.change_center) {
        twist_flag |= CubeTwist::centers;
    }

    Cube state;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            state.twist(skeleton, insert_place, skeleton.length(), twist_flag);
            state.rotate(placement);
            state.twist(this->finder.scramble_cube, twist_flag);
            state.twist(skeleton, 0, insert_place, twist_flag);
        } else {
            int twist = skeleton[insert_place - 1];
            state.twist_before(Algorithm::inverse_twist[twist], twist_flag);
            state.twist(twist, twist_flag);
        }
        this->try_insertion(insert_place, state, cycle_status);

        if (skeleton.swappable(insert_place)) {
            int twist0 = skeleton[insert_place - 1];
            int twist1 = skeleton[insert_place];
            Cube swapped_state;
            swapped_state.twist(twist0, twist_flag);
            swapped_state.twist(Algorithm::inverse_twist[twist1], twist_flag);
            swapped_state.twist(state, twist_flag);
            swapped_state.twist(twist1, twist_flag);
            swapped_state.twist(Algorithm::inverse_twist[twist0], twist_flag);
            this->try_insertion(insert_place, swapped_state, cycle_status, true);
        }
    }
}

void BruteForceFinder::Worker::search_last_corner_cycle(size_t begin, size_t end) {
    const Algorithm skeleton = this->solving_step.back().skeleton;
    static constexpr byte twist_flag = CubeTwist::corners | CubeTwist::reversed;
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
            int swapped_index = Cube::next_corner_cycle_index(
                Cube::next_corner_cycle_index(index, skeleton[insert_place]),
                Algorithm::inverse_twist[skeleton[insert_place - 1]]
            );
            this->try_last_insertion(insert_place, corner_cycle_index[swapped_index], true);
        }
    }
}

void BruteForceFinder::Worker::search_last_edge_cycle(size_t begin, size_t end) {
    const Algorithm skeleton = this->solving_step.back().skeleton;
    static constexpr byte twist_flag = CubeTwist::edges | CubeTwist::reversed;
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
            int swapped_index = Cube::next_edge_cycle_index(
                Cube::next_edge_cycle_index(index, skeleton[insert_place]),
                Algorithm::inverse_twist[skeleton[insert_place - 1]]
            );
            this->try_last_insertion(insert_place, edge_cycle_index[swapped_index], true);
        }
    }
}

void BruteForceFinder::Worker::search_last_placement(int placement, size_t begin, size_t end) {
    const Algorithm skeleton = this->solving_step.back().skeleton;
    int case_index = this->finder.center_index[Cube::inverse_center[placement]];
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
    CycleStatus cycle_status,
    bool swapped
) {
    Insertion& insertion = this->solving_step.back();
    if (swapped) {
        insertion.skeleton.swap_adjacent(insert_place);
    }
    insertion.insert_place = insert_place;
    uint32_t mask = state.mask();
    auto insert_place_mask = insertion.skeleton.get_insert_place_mask(insert_place);
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
        bool center_changed = _case.mask() & 0xf00000;
        byte twist_flag {0};
        if (corner_changed) {
            twist_flag |= CubeTwist::corners;
        }
        if (edge_changed) {
            twist_flag |= CubeTwist::edges;
        }
        if (center_changed) {
            twist_flag |= CubeTwist::centers;
        }
        Cube cube = Cube::twist(state, _case.state(), twist_flag);
        bool new_parity = parity ^ _case.has_parity();
        int new_corner_cycles = corner_changed ? cube.corner_cycles() : corner_cycles;
        int new_edge_cycles = edge_changed ? cube.edge_cycles() : edge_cycles;
        int new_placement = Cube::placement_twist(_case.rotation(), placement);
        if (
            !new_parity
            && new_corner_cycles == 0 && new_edge_cycles == 0
            && new_placement == 0
        ) {
            this->solution_found(insert_place, _case);
        } else if (
            this->finder.get_total_cycles(new_parity, new_corner_cycles, new_edge_cycles, new_placement)
            < this->finder.get_total_cycles(parity, corner_cycles, edge_cycles, placement)
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
                    && new_skeleton.length() <= this->finder.fewest_moves
                ) {
                    size_t new_end = new_skeleton.length();
                    this->solving_step.emplace_back(move(new_skeleton));
                    this->search(
                        {new_parity, new_corner_cycles, new_edge_cycles, new_placement},
                        new_begin, new_end
                    );
                    this->solving_step.pop_back();
                }
            }
        }
    }
    if (swapped) {
        this->solving_step.back().skeleton.swap_adjacent(insert_place);
    }
}

void BruteForceFinder::Worker::try_last_insertion(std::size_t insert_place, int case_index, bool swapped) {
    if (case_index == -1) {
        return;
    }
    if (swapped) {
        this->solving_step.back().skeleton.swap_adjacent(insert_place);
    }
    this->solution_found(insert_place, this->finder.cases[case_index]);
    if (swapped) {
        this->solving_step.back().skeleton.swap_adjacent(insert_place);
    }
}

void BruteForceFinder::Worker::solution_found(size_t insert_place, const Case& _case) {
    Insertion& insertion = this->solving_step.back();
    insertion.insert_place = insert_place;
    auto insert_place_mask = insertion.skeleton.get_insert_place_mask(insert_place);
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
        this->solving_step.emplace_back(insertion.skeleton.insert(algorithm, insert_place).first);
        this->update_fewest_moves();
        this->solving_step.pop_back();
    }
}

void BruteForceFinder::Worker::update_fewest_moves() {
    size_t twists = this->solving_step.back().skeleton.length();
    if (twists > this->finder.fewest_moves) {
        return;
    }
    lock_guard<mutex> lock(this->finder.fewest_moves_mutex);
    if (twists > this->finder.fewest_moves) {
        return;
    }
    if (twists < this->finder.fewest_moves) {
        this->finder.solutions.clear();
        this->finder.fewest_moves = twists;
        if (this->finder.verbose) {
            cerr << this->solving_step.back().skeleton << " (" << twists << "f)" << endl;
        }
    }
    this->finder.solutions.emplace_back(this->solving_step);
}
