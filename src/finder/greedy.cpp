#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>
#include <boost/asio.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>
#include <insertionfinder/finder/greedy.hpp>
#include "utils.hpp"
using namespace std;
using namespace InsertionFinder;
using namespace Details;


void GreedyFinder::search_core(
    const CycleStatus& cycle_status,
    const SearchParams& params
) {
    this->fewest_moves = params.search_target;
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    int placement = cycle_status.placement;
    int cycles = this->get_total_cycles(parity, corner_cycles, edge_cycles, placement);
    this->partial_solution_list.resize(cycles + 1);
    this->partial_solution_list.back().emplace_back(
        this->skeleton,
        SolvingStep {nullptr, 0, nullptr, false, cycle_status, 0}
    );
    this->partial_states = new PartialState[cycles + 1];
    for (int i = 1; i <= cycles; ++i) {
        this->partial_states[i].fewest_moves =
            max<size_t>(this->fewest_moves, this->options.greedy_threshold) - this->options.greedy_threshold;
    }
    this->additional_solution_list.resize(cycles + 1);

    size_t max_threads = params.max_threads;
    for (int depth = cycles; depth > 0; --depth) {
        auto& solution_list = this->partial_solution_list[depth];
        auto& state = this->partial_states[depth];
        auto& additional_solution_list = this->additional_solution_list[depth];
        solution_list.erase(
            remove_if(
                solution_list.begin(), solution_list.end(),
                [&state, this](const auto& x) {
                    return x.first.length() > state.fewest_moves + this->options.greedy_threshold;
                }
            ),
            solution_list.end()
        );
        sort(solution_list.begin(), solution_list.end(), [](const auto& x, const auto& y) {
            if (
                int comparison = Algorithm::compare(x.first, y.first);
                comparison < 0
            ) {
                return true;
            } else if (comparison > 0) {
                return false;
            }
            return x.second.cancellation < y.second.cancellation;
        });
        solution_list.erase(
            unique(
                solution_list.begin(), solution_list.end(),
                [](const auto& x, const auto& y) {return x.first == y.first;}
            ),
            solution_list.end()
        );

        if (this->verbose && (!solution_list.empty() || (depth & 1) == 0)) {
            cerr << "Searching depth " << depth / 2.0 << ": "
                << solution_list.size() << " case"
                << (solution_list.size() == 1 ? "" : "s")
                << '.' << endl;
        }
        boost::asio::thread_pool pool(max_threads);
        for (const auto& item: solution_list) {
            const auto& skeleton = item.first;
            const auto& step = item.second;
            auto [iter, inserted] = this->partial_solution_map.try_emplace(skeleton, step);
            if (inserted) {
                boost::asio::post(pool, [&skeleton, &step, this]() {
                    Worker(*this, skeleton, step.cycle_status, step.cancellation).search();
                });
            } else if (step.cancellation < iter->second.cancellation) {
                iter->second = step;
            }
        }
        pool.join();

        if (this->options.enable_replacement) {
            for (size_t i = 0; i < additional_solution_list.size(); ++i) {
                const auto& [skeleton, step] = additional_solution_list[i];
                if (skeleton.length() > state.fewest_moves + this->options.replacement_threshold) {
                    continue;
                }
                auto [iter, inserted] = this->partial_solution_map.try_emplace(skeleton, step);
                if (inserted) {
                    Worker(*this, skeleton, step.cycle_status, step.cancellation).search();
                } else if (step.cancellation < iter->second.cancellation) {
                    iter->second = step;
                }
            }
        }
    }

    vector<Algorithm> skeletons;
    for (const auto& [skeleton, step]: this->partial_solution_list[0]) {
        auto [iter, inserted] = this->partial_solution_map.try_emplace(skeleton, step);
        if (inserted) {
            this->partial_solution_map[skeleton] = step;
            skeletons.push_back(skeleton);
        } else if (step.cancellation < iter->second.cancellation) {
            iter->second = step;
        }
    }
    for (const Algorithm& skeleton: skeletons) {
        vector<Insertion> result({Insertion(skeleton)});
        Algorithm current_skeleton = skeleton;
        while (current_skeleton != this->skeleton) {
            const auto& step = this->partial_solution_map[current_skeleton];
            current_skeleton = *step.skeleton;
            Algorithm previous_skeleton = *step.skeleton;
            if (step.swapped) {
                previous_skeleton.swap_adjacent(step.insert_place);
            }
            result.emplace_back(previous_skeleton, step.insert_place, step.insertion);
        }
        size_t depth = result.size();
        reverse(result.begin(), result.end());
        this->solutions.push_back({move(result)});
    }
}
