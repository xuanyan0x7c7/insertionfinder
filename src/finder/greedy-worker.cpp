#include <cstddef>
#include <cstdint>
#include <mutex>
#include <utility>
#include <boost/asio.hpp>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/greedy.hpp>
#include "utils.hpp"
using namespace std;
using namespace InsertionFinder;
using namespace Details;


void GreedyFinder::Worker::search() {
    bool parity = this->cycle_status.parity;
    int corner_cycles = this->cycle_status.corner_cycles;
    int edge_cycles = this->cycle_status.edge_cycles;
    int placement = this->cycle_status.placement;
    if (!this->finder.options.enable_replacement) {
        if (!parity && corner_cycles == 1 && edge_cycles == 0 && placement == 0) {
            this->search_last_corner_cycle();
            return;
        } else if (!parity && corner_cycles == 0 && edge_cycles == 1 && placement == 0) {
            this->search_last_edge_cycle();
            return;
        } else if (
            !parity && corner_cycles == 0 && edge_cycles == 0
            && Cube::center_cycles[placement] == 1
        ) {
            this->search_last_placement(placement);
            return;
        }
    }

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
    for (size_t insert_place = 0; insert_place <= this->skeleton.length(); ++insert_place) {
        if (insert_place == 0) {
            state.twist(this->skeleton, twist_flag);
            state.rotate(placement);
            state.twist(this->finder.scramble_cube, twist_flag);
        } else {
            int twist = this->skeleton[insert_place - 1];
            state.twist_before(Algorithm::inverse_twist[twist], twist_flag);
            state.twist(twist, twist_flag);
        }
        this->try_insertion(insert_place, state);

        if (this->skeleton.swappable(insert_place)) {
            int twist0 = this->skeleton[insert_place - 1];
            int twist1 = this->skeleton[insert_place];
            Cube swapped_state;
            swapped_state.twist(twist0, twist_flag);
            swapped_state.twist(Algorithm::inverse_twist[twist1], twist_flag);
            swapped_state.twist(state, twist_flag);
            swapped_state.twist(twist1, twist_flag);
            swapped_state.twist(Algorithm::inverse_twist[twist0], twist_flag);
            this->try_insertion(insert_place, swapped_state, true);
        }
    }
}

void GreedyFinder::Worker::search_last_corner_cycle() {
    static constexpr byte twist_flag = CubeTwist::corners | CubeTwist::reversed;
    const auto& corner_cycle_index = this->finder.corner_cycle_index;

    int index = -1;
    for (size_t insert_place = 0; insert_place <= this->skeleton.length(); ++insert_place) {
        if (insert_place == 0) {
            Cube state;
            state.twist(this->finder.inverse_scramble_cube, twist_flag);
            state.twist(this->skeleton, twist_flag);
            index = state.corner_cycle_index();
        } else {
            index = Cube::next_corner_cycle_index(index, this->skeleton[insert_place - 1]);
        }
        this->try_last_insertion(insert_place, corner_cycle_index[index]);

        if (this->skeleton.swappable(insert_place)) {
            int swapped_index = Cube::next_corner_cycle_index(
                Cube::next_corner_cycle_index(index, this->skeleton[insert_place]),
                Algorithm::inverse_twist[this->skeleton[insert_place - 1]]
            );
            this->try_last_insertion(insert_place, corner_cycle_index[swapped_index], true);
        }
    }
}

void GreedyFinder::Worker::search_last_edge_cycle() {
    static constexpr byte twist_flag = CubeTwist::edges | CubeTwist::reversed;
    const auto& edge_cycle_index = this->finder.edge_cycle_index;

    int index = -1;
    for (size_t insert_place = 0; insert_place <= this->skeleton.length(); ++insert_place) {
        if (insert_place == 0) {
            Cube state;
            state.twist(this->finder.inverse_scramble_cube, twist_flag);
            state.twist(this->skeleton, twist_flag);
            index = state.edge_cycle_index();
        } else {
            index = Cube::next_edge_cycle_index(index, this->skeleton[insert_place - 1]);
        }
        this->try_last_insertion(insert_place, edge_cycle_index[index]);

        if (this->skeleton.swappable(insert_place)) {
            int swapped_index = Cube::next_edge_cycle_index(
                Cube::next_edge_cycle_index(index, this->skeleton[insert_place]),
                Algorithm::inverse_twist[this->skeleton[insert_place - 1]]
            );
            this->try_last_insertion(insert_place, edge_cycle_index[swapped_index], true);
        }
    }
}

void GreedyFinder::Worker::search_last_placement(int placement) {
    int case_index = this->finder.center_index[Cube::inverse_center[placement]];
    for (size_t insert_place = 0; insert_place <= this->skeleton.length(); ++insert_place) {
        this->try_last_insertion(insert_place, case_index);
        if (this->skeleton.swappable(insert_place)) {
            this->try_last_insertion(insert_place, case_index, true);
        }
    }
}

void GreedyFinder::Worker::try_insertion(size_t insert_place, const Cube& state, bool swapped) {
    const auto& cycle_status = this->cycle_status;
    Algorithm skeleton = this->skeleton;
    if (swapped) {
        skeleton.swap_adjacent(insert_place);
    }
    uint32_t mask = state.mask();
    auto insert_place_mask = skeleton.get_insert_place_mask(insert_place);
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    int placement = cycle_status.placement;
    int total_cycles = this->finder.get_total_cycles(parity, corner_cycles, edge_cycles, placement);

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
        int new_total_cycles = this->finder.get_total_cycles(
            new_parity, new_corner_cycles, new_edge_cycles, new_placement
        );
        if (new_total_cycles == 0) {
            this->solution_found(insert_place, swapped, _case);
        } else if (new_total_cycles < total_cycles) {
            auto& partial_solution = this->finder.partial_solution_list[new_total_cycles];
            auto& partial_state = this->finder.partial_states[new_total_cycles];
            for (const Algorithm& algorithm: _case.algorithm_list()) {
                size_t target = partial_state.fewest_moves + this->finder.options.greedy_threshold;
                if (!skeleton.is_worthy_insertion(
                    algorithm, insert_place,
                    insert_place_mask,
                    target
                )) {
                    continue;
                }
                Algorithm new_skeleton = skeleton.insert(algorithm, insert_place).first;
                new_skeleton.normalize();
                if (new_skeleton.length() > target) {
                    continue;
                }
                lock_guard<mutex> lock(partial_state.fewest_moves_mutex);
                if (new_skeleton.length() < partial_state.fewest_moves) {
                    partial_state.fewest_moves = new_skeleton.length();
                }
                size_t new_cancellation = this->cancellation
                    + this->skeleton.length() + algorithm.length() - new_skeleton.length();
                partial_solution.emplace_back(
                    move(new_skeleton),
                    SolvingStep {
                        &this->skeleton, insert_place, &algorithm, swapped,
                        CycleStatus(new_parity, new_corner_cycles, new_edge_cycles, new_placement),
                        new_cancellation
                    }
                );
            }
        } else if (this->finder.options.enable_replacement && new_total_cycles == total_cycles) {
            auto& partial_state = this->finder.partial_states[new_total_cycles];
            for (const Algorithm& algorithm: _case.algorithm_list()) {
                size_t target = partial_state.fewest_moves + this->finder.options.replacement_threshold;
                if (!skeleton.is_worthy_insertion(
                    algorithm, insert_place,
                    insert_place_mask,
                    target
                )) {
                    continue;
                }
                Algorithm new_skeleton = skeleton.insert(algorithm, insert_place).first;
                new_skeleton.normalize();
                if (new_skeleton.length() > target) {
                    continue;
                }
                lock_guard<mutex> lock(partial_state.fewest_moves_mutex);
                if (new_skeleton.length() < partial_state.fewest_moves) {
                    partial_state.fewest_moves = new_skeleton.length();
                }
                size_t new_cancellation = this->cancellation
                    + this->skeleton.length() + algorithm.length() - new_skeleton.length();
                this->finder.run_worker(
                    pool,
                    move(new_skeleton),
                    SolvingStep {
                        &this->skeleton, insert_place, &algorithm, swapped,
                        CycleStatus(new_parity, new_corner_cycles, new_edge_cycles, new_placement),
                        new_cancellation
                    }
                );
            }
        }
    }
}

void GreedyFinder::Worker::try_last_insertion(size_t insert_place, int case_index, bool swapped) {
    if (case_index != -1) {
        this->solution_found(insert_place, swapped, this->finder.cases[case_index]);
    }
}

void GreedyFinder::Worker::solution_found(size_t insert_place, bool swapped, const Case& _case) {
    Algorithm skeleton = this->skeleton;
    if (swapped) {
        skeleton.swap_adjacent(insert_place);
    }
    auto insert_place_mask = skeleton.get_insert_place_mask(insert_place);
    for (const Algorithm& algorithm: _case.algorithm_list()) {
        auto& partial_solution = this->finder.partial_solution_list.front();
        if (!skeleton.is_worthy_insertion(algorithm, insert_place, insert_place_mask, this->finder.fewest_moves)) {
            continue;
        }
        Algorithm new_skeleton = skeleton.insert(algorithm, insert_place).first;
        new_skeleton.normalize();
        if (new_skeleton.length() <= this->finder.fewest_moves) {
            lock_guard<mutex> lock(this->finder.partial_states[0].fewest_moves_mutex);
            if (new_skeleton.length() > this->finder.fewest_moves) {
                continue;
            }
            if (new_skeleton.length() < this->finder.fewest_moves) {
                partial_solution.clear();
                this->finder.fewest_moves = new_skeleton.length();
            }
            size_t new_cancellation = this->cancellation
                + this->skeleton.length() + algorithm.length() - new_skeleton.length();
            partial_solution.emplace_back(
                move(new_skeleton),
                SolvingStep {
                    &this->skeleton, insert_place, &algorithm, swapped,
                    {false, 0, 0, 0},
                    new_cancellation
                }
            );
        }
    }
}
