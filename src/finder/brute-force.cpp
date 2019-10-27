#include <cmath>
#include <algorithm>
#include <functional>
#include <thread>
#include <iostream>
#include <vector>
#include <insertionfinder/finder/finder.hpp>
#include <insertionfinder/finder/brute-force.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::BruteForceFinder;
using InsertionFinder::Cube;
using InsertionFinder::Rotation;
namespace FinderStatus = InsertionFinder::FinderStatus;


void BruteForceFinder::search_core(const SearchParams& params) {
    for (const Algorithm& skeleton: this->skeletons) {
        Cube original_cube = this->scramble_cube * skeleton;
        Cube cube = original_cube.best_placement();
        bool parity = cube.has_parity();
        int corner_cycles = cube.corner_cycles();
        int edge_cycles = cube.edge_cycles();
        Rotation placement = cube.placement();
        if (!parity && Cube::center_cycles[placement] <= 1) {
            this->result.status &= ~FinderStatus::parity_algorithms_needed;
        } else if (!this->change_parity) {
            continue;
        }
        if (corner_cycles == 0) {
            this->result.status &= ~FinderStatus::corner_cycle_algorithms_needed;
        } else if (!this->change_corner) {
            continue;
        }
        if (edge_cycles == 0) {
            this->result.status &= ~FinderStatus::edge_cycle_algorithms_needed;
        } else if (!this->change_edge) {
            continue;
        }
        if (placement == 0) {
            this->result.status &= ~FinderStatus::center_algorithms_needed;
        } else if (!this->change_center) {
            continue;
        }
        if (!parity && corner_cycles == 0 && edge_cycles == 0 && placement == 0) {
            size_t twists = skeleton.length();
            if (twists <= this->fewest_moves) {
                if (twists < this->fewest_moves) {
                    this->solutions.clear();
                    this->fewest_moves = twists;
                    if (this->verbose) {
                        std::cerr << skeleton << " (" << twists << "f)" << std::endl;
                    }
                }
                this->solutions.emplace_back(Solution({Insertion(skeleton)}));
            }
            continue;
        }

        size_t thread_count = std::min(skeleton.length() + 1, params.max_threads);
        std::vector<size_t> split_points(thread_count + 1);
        for (size_t i = 1; i <= thread_count; ++i) {
            split_points[i] = std::max(
                static_cast<size_t>(
                    (skeleton.length() + 1) * (1 - sqrt(1 - static_cast<double>(i) / thread_count)) + 0.5
                ),
                split_points[i - 1] + 1
            );
        }

        std::vector<std::thread> worker_threads;
        for (size_t i = 0; i < thread_count; ++i) {
            worker_threads.emplace_back(
                std::mem_fn(&BruteForceFinder::run_worker),
                std::ref(*this),
                skeleton,
                CycleStatus {parity, corner_cycles, edge_cycles, placement},
                split_points[i], split_points[i + 1] - 1
            );
        }
        for (std::thread& thread: worker_threads) {
            thread.join();
        }
    }
}
