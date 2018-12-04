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
    this->partial_solutions.resize(cycles + 1);
    this->partial_solutions.back().emplace(
        this->skeleton,
        SolvingStep{{nullptr, 0, nullptr, false}, cycle_status}
    );
    this->partial_states = new PartialState[cycles];
    for (int i = 0; i < cycles; ++i) {
        this->partial_states[i].fewest_moves =
            max<size_t>(this->fewest_moves, this->threshold) - this->threshold;
    }

    size_t max_threads = params.max_threads;
    for (int depth = cycles; depth > 0; --depth) {
        vector<Skeleton> skeletons;
        for (const auto& [skeleton, step]: this->partial_solutions[depth]) {
            if (skeletons.empty() || skeleton != *(skeletons.back().skeleton)) {
                skeletons.push_back({&skeleton, &step.cycle_status});
            }
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
}


void GreedyFinder::run_worker(
    const vector<GreedyFinder::Skeleton>& skeletons,
    size_t start, size_t step
) {
    for (size_t index = start; index < skeletons.size(); index += step) {
        const auto& [skeleton, cycle_status] = skeletons[index];
        Worker(*this, *skeleton, *cycle_status).search();
    }
}
