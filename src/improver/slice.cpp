#include <mutex>
#include <utility>
#include <vector>
#include <range/v3/all.hpp>
#include <boost/asio/thread_pool.hpp>
#include <insertionfinder/improver/improver.hpp>
#include <insertionfinder/improver/slice.hpp>
using InsertionFinder::Algorithm;
using InsertionFinder::Insertion;
using InsertionFinder::SliceImprover;


void SliceImprover::search_core(const SearchParams& params) {
    boost::asio::thread_pool pool(params.max_threads);
    this->run_worker(pool, Algorithm(this->skeleton), SolvingStep {nullptr, 0, nullptr, false, 0, 0});
    pool.join();

    for (const Algorithm& algorithm: this->partial_solution_list) {
        std::vector<Insertion> result({Insertion(algorithm)});
        const Algorithm* current_skeleton = &algorithm;
        while (*current_skeleton != this->skeleton) {
            const SolvingStep& step = this->partial_solution_map.at(*current_skeleton);
            current_skeleton = step.skeleton;
            Algorithm previous_skeleton = *step.skeleton;
            if (step.swapped) {
                previous_skeleton.swap_adjacent(step.insert_place);
            }
            result.emplace_back(std::move(previous_skeleton), step.insert_place, step.insertion);
        }
        ranges::actions::reverse(result);
        this->solutions.emplace_back(move(result));
    }
}

void SliceImprover::run_worker(boost::asio::thread_pool& pool, Algorithm&& skeleton, const SolvingStep& step) {
    std::lock_guard<std::mutex> lock(this->worker_mutex);
    auto [iter, inserted] = this->partial_solution_map.try_emplace(std::move(skeleton), step);
    const Algorithm& old_skeleton = iter->first;
    SolvingStep& old_step = iter->second;
    if (inserted) {
        boost::asio::post(pool, [&]() {
            Worker(*this, pool, old_skeleton, old_step.placement, old_step.cancellation).search();
        });
    } else if (step.cancellation < old_step.cancellation) {
        old_step = step;
    }
}
