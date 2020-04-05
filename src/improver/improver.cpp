#include <cstddef>
#include <chrono>
#include <range/v3/all.hpp>
#include <insertionfinder/insertion.hpp>
#include <insertionfinder/improver/improver.hpp>
using std::size_t;
using InsertionFinder::Improver;
using InsertionFinder::Insertion;
using InsertionFinder::Solution;


void Improver::search(const SearchParams& params) {
    auto begin = std::chrono::high_resolution_clock::now();
    this->search_core(params);
    for (Solution& solution: this->solutions) {
        size_t cancellation = this->skeleton.length();
        for (const Insertion& insertion: solution.insertions) {
            cancellation += insertion.insertion->length();
        }
        cancellation -= solution.final_solution.length();
        solution.cancellation = cancellation;
    }
    ranges::sort(
        this->solutions,
        [](const Solution& x, const Solution& y) {return x.cancellation < y.cancellation;}
    );
    auto end = std::chrono::high_resolution_clock::now();
    this->result.duration = (end - begin).count();
}
