#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include <cube.hpp>
#include <finder/finder.hpp>
#include <finder/greedy.hpp>
using namespace std;
using namespace InsertionFinder;


void GreedyFinder::search_core(
    const CycleStatus& cycle_status,
    size_t max_threads
) {
    bool parity = cycle_status.parity;
    int corner_cycles = cycle_status.corner_cycles;
    int edge_cycles = cycle_status.edge_cycles;
    int placement = cycle_status.placement;
    int cycles = (Cube::center_cycles[placement] > 1 ? 0 : parity)
        + corner_cycles + edge_cycles + Cube::center_cycles[placement];
    this->partial_solutions.resize(cycles + 1);
    this->partial_solutions.back().push_back({
        {{make_shared<Algorithm>(this->skeleton)}},
        cycle_status
    });
    this->partial_states = new PartialState[cycles];
    for (int i = 0; i < cycles; ++i) {
        this->partial_states[i].fewest_moves
            = numeric_limits<size_t>::max() - this->threshold;
    }

    for (int depth = cycles; depth > 0; --depth) {
        auto& partial_solution = this->partial_solutions.back();
        sort(
            partial_solution.begin(), partial_solution.end(),
            [](const auto& x, const auto& y) {
                return x.steps.back().skeleton->length()
                    < y.steps.back().skeleton->length();
            }
        );
        if (this->verbose) {
            cerr << "Searching depth " << depth << ": "
                << partial_solution.size() << " case"
                << (partial_solution.size() == 1 ? "" : "s")
                << '.' << endl;
        }
        vector<thread> worker_threads;
        for (size_t i = 0; i < max_threads; ++i) {
            worker_threads.emplace_back(
                mem_fn(&GreedyFinder::run_worker),
                ref(*this),
                i, max_threads
            );
        }
        for (thread& thread: worker_threads) {
            thread.join();
        }
        this->partial_solutions.pop_back();
    }

    for (const auto& solution: this->partial_solutions.front()) {
        vector<Insertion> insertions;
        for (const auto& step: solution.steps) {
            insertions.push_back({
                *step.skeleton,
                step.insert_place,
                step.insertion
            });
        }
        this->solutions.push_back({move(insertions)});
    }
}


void GreedyFinder::run_worker(size_t start, size_t step) {
    for (
        size_t index = start;
        index < this->partial_solutions.back().size();
        index += step
    ) {
        Worker(*this, this->partial_solutions.back()[index]).search();
    }
}
