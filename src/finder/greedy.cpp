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
    int cycles = get_total_cycles(parity, corner_cycles, edge_cycles, placement);
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
            if (
                auto iter = this->partial_solution_map.find(skeleton);
                iter == this->partial_solution_map.end()
            ) {
                this->partial_solution_map[skeleton] = step;
                skeletons.push_back({&skeleton, &step.cycle_status, step.cancellation});
            } else {
                if (step.cancellation < iter->second.cancellation) {
                    iter->second = step;
                    auto place = find_if(
                        skeletons.begin(), skeletons.end(),
                        [iter](const auto& x) {return *x.skeleton == iter->first;}
                    );
                    *place = {&skeleton, &step.cycle_status, step.cancellation};
                } else {
                    continue;
                }
            }
        }
        sort(
            skeletons.begin(), skeletons.end(),
            [](const auto& x, const auto& y) {
                return x.skeleton->length() < y.skeleton->length();
            }
        );
        if (this->verbose && (!skeletons.empty() || (depth & 1) == 0)) {
            cerr << "Searching depth " << depth / 2.0 << ": "
                << skeletons.size() << " case"
                << (skeletons.size() == 1 ? "" : "s")
                << '.' << endl;
        }
        boost::asio::thread_pool pool(max_threads);
        for (const auto& _skeleton: skeletons) {
            boost::asio::post(pool, [&, this]() {
                Worker(*this, *_skeleton.skeleton, *_skeleton.cycle_status, _skeleton.cancellation).search();
            });
        }
        pool.join();
    }

    vector<Algorithm> skeletons;
    for (const auto& [skeleton, step]: this->partial_solution_list[0]) {
        auto iter = this->partial_solution_map.find(skeleton);
        if (iter == this->partial_solution_map.end()) {
            this->partial_solution_map[skeleton] = step;
            skeletons.push_back(skeleton);
        } else if (step.cancellation < iter->second.cancellation) {
            iter->second = step;
        }
    }
    for (const Algorithm& skeleton: skeletons) {
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
        reverse(result.begin(), result.end());
        this->solutions.push_back({move(result)});
    }
}
