#include <cmath>
#include <algorithm>
#include <functional>
#include <thread>
#include <vector>
#include <cube.hpp>
#include <finder/finder.hpp>
#include <finder/brute-force.hpp>
using namespace std;
using namespace InsertionFinder;


Finder::Status BruteForceFinder::search_core(size_t max_threads) {
    Cube original_cube = this->scramble_cube;
    original_cube.twist(this->skeleton);
    Cube cube = original_cube.best_placement();
    bool parity = cube.has_parity();
    int corner_cycles = cube.corner_cycles();
    int edge_cycles = cube.edge_cycles();
    int placement = cube.placement();
    if (!parity && corner_cycles == 0 && edge_cycles == 0 && placement == 0) {
        return Finder::Status::SUCCESS_SOLVED;
    } else if ((parity || Cube::placement_parity(placement)) && !this->change_parity) {
        return Finder::Status::FAILURE_PARITY_ALGORITHMS_NEEDED;
    } else if (corner_cycles && !this->change_corner) {
        return Finder::Status::FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED;
    } else if (edge_cycles && !this->change_edge) {
        return Finder::Status::FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED;
    } else if (placement && !this->change_center) {
        return Finder::Status::FAILURE_CENTER_ALGORITHMS_NEEDED;
    }

    size_t thread_count = min({
        static_cast<size_t>(thread::hardware_concurrency()),
        this->skeleton.length() + 1,
        max_threads
    });
    vector<size_t> split_points(thread_count + 1);
    for (size_t i = 1; i <= thread_count; ++i) {
        split_points[i] = max(
            static_cast<size_t>(
                (this->skeleton.length() + 1) * (
                    1 - sqrt(1 - static_cast<double>(i) / thread_count)
                ) + 0.5
            ),
            split_points[i - 1] + 1
        );
    }

    vector<thread> worker_threads;
    CycleStatus cycle_status = {parity, corner_cycles, edge_cycles, placement};
    for (size_t i = 0; i < thread_count; ++i) {
        worker_threads.emplace_back(
            mem_fn(&BruteForceFinder::run_worker),
            ref(*this),
            cycle_status,
            split_points[i], split_points[i + 1] - 1
        );
    }
    for (thread& thread: worker_threads) {
        thread.join();
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

    return Finder::Status::SUCCESS;
}


void BruteForceFinder::run_worker(
    const CycleStatus& cycle_status,
    size_t begin, size_t end
) {
    BruteForceFinder::Worker worker(*this);
    worker.search(cycle_status, begin, end);
}
