#include <chrono>
#include <range/v3/all.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/improver/improver.hpp>
using std::size_t;
using InsertionFinder::Improver;
using InsertionFinder::Solution;


void Improver::search(const SearchParams& params) {
    auto begin = std::chrono::high_resolution_clock::now();
    this->search_core(params);
    for (Solution& solution: this->solutions) {
        size_t cancellation = solution.insertions.front().skeleton.length();
        for (size_t i = 0; i < solution.insertions.size() - 1; ++i) {
            cancellation += solution.insertions[i].insertion->length();
        }
        cancellation -= solution.insertions.back().skeleton.length();
        solution.cancellation = cancellation;
    }
    ranges::sort(
        this->solutions,
        [](const Solution& x, const Solution& y) {return x.cancellation < y.cancellation;}
    );
    auto end = std::chrono::high_resolution_clock::now();
    this->result.duration = (end - begin).count();
}
