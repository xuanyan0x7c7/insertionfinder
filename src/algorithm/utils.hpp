#pragma once
#include <cstdint>

namespace InsertionFinder::Details {
    constexpr std::uint32_t twist_mask(std::int_fast8_t twist) {
        return 1 << twist | 1 << (24 + (twist >> 2));
    }

    constexpr std::int_fast8_t transform_twist(const std::int_fast8_t* transform, std::int_fast8_t twist) {
        return transform[twist >> 3] << 2 ^ (twist & 7);
    }

    constexpr std::int_fast8_t rotation_permutation[24][3] = {
        {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
        {5, 2, 0}, {2, 4, 0}, {4, 3, 0}, {3, 5, 0},
        {1, 2, 5}, {1, 4, 2}, {1, 3, 4}, {1, 5, 3},
        {4, 2, 1}, {3, 4, 1}, {5, 3, 1}, {2, 5, 1},
        {2, 1, 4}, {4, 1, 3}, {3, 1, 5}, {5, 1, 2},
        {3, 0, 4}, {5, 0, 3}, {2, 0, 5}, {4, 0, 2}
    };
};
