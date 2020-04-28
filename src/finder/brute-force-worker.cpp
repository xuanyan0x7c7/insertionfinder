#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/case.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/twist.hpp>
#include <insertionfinder/finder/brute-force.hpp>
#include "utils.hpp"
using std::size_t;
using std::uint64_t;
using InsertionFinder::Algorithm;
using InsertionFinder::BruteForceFinder;
using InsertionFinder::Case;
using InsertionFinder::Cube;
using InsertionFinder::Insertion;
using InsertionFinder::InsertionAlgorithm;
using InsertionFinder::Rotation;
using InsertionFinder::Twist;
namespace CubeTwist = InsertionFinder::CubeTwist;
namespace Details = InsertionFinder::Details;


namespace {
    bool not_searched(const Algorithm& algorithm, size_t insert_place, size_t new_begin, bool swapped) {
        if (swapped || insert_place < 2 || algorithm.swappable(insert_place - 1)) {
            return new_begin >= insert_place;
        } else {
            return new_begin >= insert_place - 1;
        }
    }
};


void BruteForceFinder::Worker::search(CycleStatus cycle_status, size_t begin, size_t end) {
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    Rotation placement = cycle_status.placement;
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
    std::byte twist_flag{0};
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
            state.rotate(placement, twist_flag);
            state.twist(this->finder.scramble_cube, twist_flag);
            state.twist(skeleton, 0, insert_place, twist_flag);
        } else {
            Twist twist = skeleton[insert_place - 1];
            state.twist_before(twist.inverse(), twist_flag);
            state.twist(twist, twist_flag);
        }
        this->try_insertion(insert_place, state, cycle_status);

        if (skeleton.swappable(insert_place)) {
            Twist twist0 = skeleton[insert_place - 1];
            Twist twist1 = skeleton[insert_place];
            Cube swapped_state;
            swapped_state.twist(twist0, twist_flag);
            swapped_state.twist(twist1.inverse(), twist_flag);
            swapped_state.twist(state, twist_flag);
            swapped_state.twist(twist1, twist_flag);
            swapped_state.twist(twist0.inverse(), twist_flag);
            this->try_insertion(insert_place, swapped_state, cycle_status, true);
        }
    }
}

void BruteForceFinder::Worker::search_last_corner_cycle(size_t begin, size_t end) {
    const Algorithm skeleton = this->solving_step.back().skeleton;
    static constexpr std::byte twist_flag = CubeTwist::corners;
    const int* corner_cycle_index = this->finder.corner_cycle_index;

    int index = -1;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            Cube state;
            state.twist_inverse(skeleton, 0, insert_place, twist_flag);
            state.twist(this->finder.inverse_scramble_cube, twist_flag);
            state.twist_inverse(skeleton, insert_place, skeleton.length(), twist_flag);
            index = state.corner_cycle_index();
        } else {
            index = Cube::next_corner_cycle_index(index, skeleton[insert_place - 1]);
        }
        this->try_last_insertion(insert_place, corner_cycle_index[index]);

        if (skeleton.swappable(insert_place)) {
            int swapped_index = Cube::next_corner_cycle_index(
                index,
                {skeleton[insert_place], skeleton[insert_place - 1].inverse()}
            );
            this->try_last_insertion(insert_place, corner_cycle_index[swapped_index], true);
        }
    }
}

void BruteForceFinder::Worker::search_last_edge_cycle(size_t begin, size_t end) {
    const Algorithm skeleton = this->solving_step.back().skeleton;
    static constexpr std::byte twist_flag = CubeTwist::edges;
    const int* edge_cycle_index = this->finder.edge_cycle_index;

    int index = -1;
    for (size_t insert_place = begin; insert_place <= end; ++insert_place) {
        if (insert_place == begin) {
            Cube state;
            state.twist_inverse(skeleton, 0, insert_place, twist_flag);
            state.twist(this->finder.inverse_scramble_cube, twist_flag);
            state.twist_inverse(skeleton, insert_place, skeleton.length(), twist_flag);
            index = state.edge_cycle_index();
        } else {
            index = Cube::next_edge_cycle_index(index, skeleton[insert_place - 1]);
        }
        this->try_last_insertion(insert_place, edge_cycle_index[index]);

        if (skeleton.swappable(insert_place)) {
            int swapped_index = Cube::next_edge_cycle_index(
                index,
                {skeleton[insert_place], skeleton[insert_place - 1].inverse()}
            );
            this->try_last_insertion(insert_place, edge_cycle_index[swapped_index], true);
        }
    }
}

void BruteForceFinder::Worker::search_last_placement(Rotation placement, size_t begin, size_t end) {
    const Algorithm skeleton = this->solving_step.back().skeleton;
    int case_index = this->finder.center_index[placement.inverse()];
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
    uint64_t mask = state.mask();
    bool corner_solved = !(mask & 0xff);
    bool edge_solved = !(mask & 0xfff00);
    bool center_solved = !(mask & 0x3f00000);
    std::byte case_flag {0};
    if (!corner_solved) {
        case_flag |= CubeTwist::corners;
    }
    if (!edge_solved) {
        case_flag |= CubeTwist::edges;
    }
    if (!center_solved) {
        case_flag |= CubeTwist::centers;
    }
    auto insert_place_mask = insertion.skeleton.get_insert_place_mask(insert_place);
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    Rotation placement = cycle_status.placement;

    for (const Case& _case: this->finder.cases) {
        if (Details::bitcount_less_than_2(mask & _case.get_mask() & 0xffffffff)) {
            continue;
        }
        bool corner_changed = _case.get_mask() & 0xff;
        bool edge_changed = _case.get_mask() & 0xfff00;
        bool center_changed = _case.get_mask() & 0x3f00000;
        std::byte twist_flag {0};
        if (corner_changed) {
            twist_flag |= CubeTwist::corners;
        }
        if (edge_changed) {
            twist_flag |= CubeTwist::edges;
        }
        if (center_changed) {
            twist_flag |= CubeTwist::centers;
        }
        Cube cube = Cube::twist(state, _case.get_state(), case_flag, twist_flag);
        bool new_parity = parity ^ _case.has_parity();
        int new_corner_cycles = corner_changed
            ? (corner_solved ? _case.get_corner_cycles() : cube.corner_cycles())
            : corner_cycles;
        int new_edge_cycles = edge_changed ? (edge_solved ? _case.get_edge_cycles() : cube.edge_cycles()) : edge_cycles;
        Rotation new_placement = _case.get_placement() * placement;
        if (!new_parity && new_corner_cycles == 0 && new_edge_cycles == 0 && new_placement == 0) {
            this->solution_found(insert_place, _case);
        } else if (
            this->finder.get_total_cycles(new_parity, new_corner_cycles, new_edge_cycles, new_placement)
            < this->finder.get_total_cycles(parity, corner_cycles, edge_cycles, placement)
        ) {
            for (const InsertionAlgorithm& algorithm: _case.algorithm_list()) {
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
                    this->solving_step.emplace_back(std::move(new_skeleton));
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

void BruteForceFinder::Worker::try_last_insertion(size_t insert_place, int case_index, bool swapped) {
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
    for (const InsertionAlgorithm& algorithm: _case.algorithm_list()) {
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
    std::lock_guard<std::mutex> lock(this->finder.fewest_moves_mutex);
    if (twists > this->finder.fewest_moves) {
        return;
    }
    if (twists < this->finder.fewest_moves) {
        this->finder.solutions.clear();
        this->finder.fewest_moves = twists;
        if (this->finder.verbose) {
            std::cerr << this->solving_step.back().skeleton << " (" << twists << "f)" << std::endl;
        }
    }
    this->finder.solutions.emplace_back(
        this->solving_step.back().skeleton,
        std::vector<Insertion>(this->solving_step.cbegin(), this->solving_step.cend() - 1)
    );
}
