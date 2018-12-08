#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/finder/finder.hpp>
#include <insertionfinder/finder/greedy.hpp>
using namespace std;
using namespace InsertionFinder;


void GreedyFinder::search_core(
    const CycleStatus& cycle_status,
    const SearchParams& params
) {
    this->fewest_moves = params.search_target;
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    int placement = cycle_status.placement;
    int cycles = (Cube::center_cycles[placement] > 1 ? 0 : parity)
        + corner_cycles + edge_cycles + Cube::center_cycles[placement];
    this->partial_solution_list.resize(cycles + 1);
    this->partial_solution_list.back().push_back({
        this->skeleton,
        SolvingStep{nullptr, 0, nullptr, false, cycle_status, 0}
    });
    this->partial_states = new PartialState[cycles + 1];
    for (int i = 1; i <= cycles; ++i) {
        this->partial_states[i].fewest_moves =
            max<size_t>(this->fewest_moves, this->threshold) - this->threshold;
    }

    size_t max_threads = params.max_threads;
    for (int depth = cycles; depth > 0; --depth) {
        vector<Skeleton> skeletons;
        for (const auto& [skeleton, step]: this->partial_solution_list[depth]) {
            if (skeleton.length() > this->partial_states[depth].fewest_moves + this->threshold) {
                continue;
            }
            auto iter = this->partial_solution_map.find(skeleton);
            if (iter == this->partial_solution_map.end()) {
                this->partial_solution_map[skeleton] = step;
            } else {
                if (step.cancellation < iter->second.cancellation) {
                    iter->second = step;
                } else {
                    continue;
                }
            }
            skeletons.push_back({&skeleton, &step.cycle_status, step.cancellation});
        }
        sort(
            skeletons.begin(), skeletons.end(),
            [](const auto& x, const auto& y) {
                return x.skeleton->length() < y.skeleton->length();
            }
        );
        if (this->verbose) {
            cerr << "Searching depth " << depth << ": "
                << skeletons.size() << " case"
                << (skeletons.size() == 1 ? "" : "s")
                << '.' << endl;
        }
        vector<thread> worker_threads;
        for (size_t i = 0; i < max_threads; ++i) {
            worker_threads.emplace_back(
                mem_fn(&GreedyFinder::run_worker),
                ref(*this),
                skeletons, i, max_threads
            );
        }
        for (thread& thread: worker_threads) {
            thread.join();
        }
    }

    for (const auto& [skeleton, step]: this->partial_solution_list[0]) {
        auto iter = this->partial_solution_map.find(skeleton);
        if (iter == this->partial_solution_map.end()) {
            this->partial_solution_map[skeleton] = step;
        } else {
            if (step.cancellation < iter->second.cancellation) {
                iter->second = step;
            } else {
                continue;
            }
        }
        vector<Insertion> result({{skeleton, 0, nullptr}});
        Algorithm current_skeleton = skeleton;
        while (current_skeleton != this->skeleton) {
            const auto& step = this->partial_solution_map[current_skeleton];
            current_skeleton = *step.skeleton;
            Algorithm previous_skeleton = *step.skeleton;
            if (step.swapped) {
                previous_skeleton.swap_adjacent(step.insert_place);
            }
            result.push_back({previous_skeleton, step.insert_place, step.insertion});
        }
        size_t depth = result.size();
        for (size_t i = 0; i < depth >> 1; ++i) {
            swap(result[i], result[depth - 1 - i]);
        }
        this->solutions.push_back({move(result)});
    }
}


void GreedyFinder::run_worker(
    const vector<GreedyFinder::Skeleton>& skeletons,
    size_t start, size_t step
) {
    for (size_t index = start; index < skeletons.size(); index += step) {
        const auto& [skeleton, cycle_status, cancellation] = skeletons[index];
        Worker(*this, *skeleton, *cycle_status, cancellation).search();
    }
}
