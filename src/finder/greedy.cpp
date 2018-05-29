#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <cube.hpp>
#include <finder/finder.hpp>
#include <finder/greedy.hpp>
using namespace std;
using namespace InsertionFinder;


Finder::Status GreedyFinder::search_core(size_t max_threads) {
    Cube original_cube = this->scramble_cube;
    original_cube.twist(this->skeleton);
    Cube cube = original_cube.best_placement();
    bool parity = cube.has_parity();
    int corner_cycles = cube.corner_cycles();
    int edge_cycles = cube.edge_cycles();
    int placement = cube.placement();
    if (!parity && corner_cycles == 0 && edge_cycles == 0 && placement == 0) {
        return Finder::Status::SUCCESS_SOLVED;
    } else if ((parity || Cube::center_cycles[placement] > 1) && !this->change_parity) {
        return Finder::Status::FAILURE_PARITY_ALGORITHMS_NEEDED;
    } else if (corner_cycles && !this->change_corner) {
        return Finder::Status::FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED;
    } else if (edge_cycles && !this->change_edge) {
        return Finder::Status::FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED;
    } else if (placement && !this->change_center) {
        return Finder::Status::FAILURE_CENTER_ALGORITHMS_NEEDED;
    }

    int cycles = (Cube::center_cycles[placement] > 1 ? 0 : parity)
        + corner_cycles + edge_cycles + Cube::center_cycles[placement];
    this->partial_solutions.resize(cycles + 1);
    this->partial_solutions.back().push_back({
        {{make_shared<Algorithm>(this->skeleton)}},
        {parity, corner_cycles, edge_cycles, placement}
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
                return x.steps.back().skeleton->length() < y.steps.back().skeleton->length();
            }
        );
    }

    for (const auto& solution: this->partial_solutions.front()) {
        vector<Insertion> insertions;
        for (const auto& step: solution.steps) {
            insertions.push_back({*step.skeleton, step.insert_place, step.insertion});
        }
        this->solutions.push_back({move(insertions)});
    }

    for (auto& solution: this->solutions) {
        size_t cancellation = solution.insertions.front().skeleton.length();
        for (size_t i = 0; i < solution.insertions.size() - 1; ++i) {
            cancellation += solution.insertions[i].insertion->length();
        }
        cancellation -= solution.insertions.back().skeleton.length();
        solution.cancellation = cancellation;
    }
    sort(
        this->solutions.begin(), this->solutions.end(),
        [](const auto& x, const auto& y) {return x.cancellation < y.cancellation;}
    );
    if (!this->solutions.empty()) {
        this->fewest_moves = this->solutions.front().insertions.back().skeleton.length();
    }

    return Finder::Status::SUCCESS;
}
