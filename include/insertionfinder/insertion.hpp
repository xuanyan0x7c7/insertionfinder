#pragma once
#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>
#include <insertionfinder/algorithm.hpp>

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
        void print(std::ostream& out, std::size_t index) const;
    };

    struct MergedInsertion {
        Algorithm skeleton;
        Algorithm final_solution;
        std::vector<std::pair<std::size_t, std::vector<std::size_t>>> insert_places;
        std::vector<Algorithm> insertions;
        void print(std::ostream& out, std::size_t start_index) const;
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
        std::vector<MergedInsertion> merge_insertions(const Algorithm& skeleton) const;
        void print(std::ostream& _opaque_pthread_condattr_t, const Algorithm& skeleton) const;
    };
};
