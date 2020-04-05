#pragma once
#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>
#include <insertionfinder/algorithm.hpp>

namespace InsertionFinder {
    class Insertion;
    class Solution;
};
std::ostream& operator<<(std::ostream&, const InsertionFinder::Insertion&);
std::ostream& operator<<(std::ostream&, const InsertionFinder::Solution&);

namespace InsertionFinder {
    struct Insertion {
        Algorithm skeleton;
        std::size_t insert_place;
        const Algorithm* insertion;
        explicit Insertion(
            const Algorithm& skeleton,
            std::size_t insert_place = 0,
            const Algorithm* insertion = nullptr
        ): skeleton(skeleton), insert_place(insert_place), insertion(insertion) {}
        explicit Insertion(
            Algorithm&& skeleton,
            std::size_t insert_place = 0,
            const Algorithm* insertion = nullptr
        ): skeleton(std::move(skeleton)), insert_place(insert_place), insertion(insertion) {}
        void print(std::ostream&) const;
        void print(std::ostream&, std::size_t index) const;
    };

    struct Solution {
        Algorithm final_solution;
        std::vector<Insertion> insertions;
        std::size_t cancellation = 0;
        Solution(const Algorithm& final_solution): final_solution(final_solution) {}
        Solution(const Algorithm& final_solution, const std::vector<Insertion>& insertions):
            final_solution(final_solution), insertions(insertions) {}
        Solution(const Algorithm& final_solution, std::vector<Insertion>&& insertions):
            final_solution(final_solution), insertions(std::move(insertions)) {}
    };
};
