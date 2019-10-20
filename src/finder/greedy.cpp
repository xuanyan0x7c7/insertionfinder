#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>
#include <boost/asio/thread_pool.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>
#include <insertionfinder/finder/greedy.hpp>
#include "utils.hpp"
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Cube;
using InsertionFinder::GreedyFinder;
namespace Details = InsertionFinder::Details;


void GreedyFinder::search_core(const SearchParams& params) {
    size_t max_threshold = std::max<size_t>(this->options.greedy_threshold, this->options.replacement_threshold);
    this->partial_states[0].fewest_moves = std::numeric_limits<size_t>::max() - max_threshold;
    for (const Algorithm& skeleton: this->skeletons) {
        Cube original_cube = this->scramble_cube * skeleton;
        Cube cube = original_cube.best_placement();
        bool parity = cube.has_parity();
        int corner_cycles = cube.corner_cycles();
        int edge_cycles = cube.edge_cycles();
        int placement = cube.placement();
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
        int cycles = this->get_total_cycles(parity, corner_cycles, edge_cycles, placement);
        if (cycles >= this->partial_states.size()) {
            int previous_cycles = this->partial_states.size() - 1;
            this->partial_solution_list.resize(cycles + 1);
            this->partial_states.resize(cycles + 1);
            for (int depth = previous_cycles; ++depth <= cycles;) {
                this->partial_states[depth].fewest_moves = std::numeric_limits<size_t>::max() - max_threshold;
            }
        }
        if (this->partial_states[cycles].fewest_moves > skeleton.length()) {
            this->partial_states[cycles].fewest_moves = skeleton.length();
        }
        this->partial_solution_list[cycles].emplace_back(
            skeleton,
            SolvingStep {nullptr, 0, nullptr, false, CycleStatus(parity, corner_cycles, edge_cycles, placement), 0}
        );
    }

    for (int depth = this->partial_states.size(); --depth > 0;) {
        auto& solution_list = this->partial_solution_list[depth];
        auto& state = this->partial_states[depth];
        solution_list.erase(
            std::remove_if(
                solution_list.begin(), solution_list.end(),
                [&state, this](const auto& x) {
                    return x.first.length() > state.fewest_moves + this->options.greedy_threshold;
                }
            ),
            solution_list.end()
        );
        std::sort(solution_list.begin(), solution_list.end(), [](const auto& x, const auto& y) {
            if (int comparison = Algorithm::compare(x.first, y.first); comparison < 0) {
                return true;
            } else if (comparison > 0) {
                return false;
            }
            return x.second.cancellation < y.second.cancellation;
        });
        solution_list.erase(
            std::unique(
                solution_list.begin(), solution_list.end(),
                [](const auto& x, const auto& y) {return x.first == y.first;}
            ),
            solution_list.end()
        );

        if (this->verbose && (!solution_list.empty() || (depth & 1) == 0)) {
            std::cerr << "Searching depth " << depth / 2.0 << ": "
                << solution_list.size() << " case"
                << (solution_list.size() == 1 ? "" : "s")
                << '.' << std::endl;
        }
        boost::asio::thread_pool pool(params.max_threads);
        for (auto& [skeleton, step]: solution_list) {
            this->run_worker(pool, std::move(skeleton), step);
        }
        pool.join();
    }

    std::vector<const Algorithm*> skeletons;
    for (auto& [skeleton, step]: this->partial_solution_list[0]) {
        auto [iter, inserted] = this->partial_solution_map.try_emplace(std::move(skeleton), step);
        const Algorithm& old_skeleton = iter->first;
        auto& old_step = iter->second;
        if (inserted) {
            skeletons.push_back(&old_skeleton);
        } else if (step.cancellation < old_step.cancellation) {
            old_step = step;
        }
    }
    for (const Algorithm* current_skeleton: skeletons) {
        std::vector<Insertion> result({Insertion(*current_skeleton)});
        while (std::all_of(
            this->skeletons.cbegin(), this->skeletons.cend(),
            [&](const Algorithm& x) {return x != *current_skeleton;}
        )) {
            const auto& step = this->partial_solution_map[*current_skeleton];
            current_skeleton = step.skeleton;
            Algorithm previous_skeleton = *step.skeleton;
            if (step.swapped) {
                previous_skeleton.swap_adjacent(step.insert_place);
            }
            result.emplace_back(std::move(previous_skeleton), step.insert_place, step.insertion);
        }
        std::reverse(result.begin(), result.end());
        this->solutions.emplace_back(move(result));
    }
}

void GreedyFinder::run_worker(boost::asio::thread_pool& pool, Algorithm&& skeleton, const SolvingStep& step) {
    std::lock_guard<std::mutex> lock(this->worker_mutex);
    auto [iter, inserted] = this->partial_solution_map.try_emplace(std::move(skeleton), step);
    const auto& old_skeleton = iter->first;
    auto& old_step = iter->second;
    if (inserted) {
        boost::asio::post(pool, [&]() {
            Worker(*this, pool, old_skeleton, old_step.cycle_status, old_step.cancellation).search();
        });
    } else if (step.cancellation < old_step.cancellation) {
        old_step = step;
    }
}
