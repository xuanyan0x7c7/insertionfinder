#pragma once
#include <cstdint>
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
    };

    struct Solution {
        std::vector<Insertion> insertions;
        std::size_t cancellation = 0;
        Solution(const std::vector<Insertion>& insertions): insertions(insertions) {}
        Solution(std::vector<Insertion>&& insertions): insertions(std::move(insertions)) {}
    };
};
