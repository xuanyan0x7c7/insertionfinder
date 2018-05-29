#include <cmath>
#include <algorithm>
#include <functional>
#include <thread>
#include <vector>
#include <finder/finder.hpp>
#include <finder/brute-force.hpp>
using namespace std;
using namespace InsertionFinder;


void BruteForceFinder::search_core(
    const CycleStatus& cycle_status,
    size_t max_threads
) {
    size_t thread_count = min(this->skeleton.length() + 1, max_threads);
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
}
