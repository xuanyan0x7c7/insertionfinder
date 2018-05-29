#include <algorithm>
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

    for (int depth = cycles; depth > 0; --depth) {
        for (const auto& solving_step: this->partial_solutions.back()) {
            Worker(*this, solving_step).search();
        }
        this->partial_solutions.pop_back();
        sort(
            this->partial_solutions.back().begin(),
            this->partial_solutions.back().end(),
            [](const auto& x, const auto& y) {
                return x.steps.back().skeleton->length()
                    < y.steps.back().skeleton->length();
            }
        );
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

    if (!this->solutions.empty()) {
        this->fewest_moves = this->solutions.front().insertions.back().skeleton.length();
    }
}
