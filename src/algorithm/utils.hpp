#pragma once
#include <cstdint>
#include <functional>

namespace InsertionFinder::Details {
    constexpr std::uint32_t twist_mask(std::uint_fast8_t twist) {
        return 1 << twist | 1 << (24 + (twist >> 2));
    }

    constexpr std::uint_fast8_t rotation_permutation[24][3] = {
        {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
        {5, 2, 0}, {2, 4, 0}, {4, 3, 0}, {3, 5, 0},
        {1, 2, 5}, {1, 4, 2}, {1, 3, 4}, {1, 5, 3},
        {4, 2, 1}, {3, 4, 1}, {5, 3, 1}, {2, 5, 1},
        {2, 1, 4}, {4, 1, 3}, {3, 1, 5}, {5, 1, 2},
        {3, 0, 4}, {5, 0, 3}, {2, 0, 5}, {4, 0, 2}
    };

    constexpr std::uint_fast8_t rotate_twist(int rotation, std::uint_fast8_t twist) {
        const std::uint_fast8_t* transform = rotation_permutation[rotation];
        return transform[twist >> 3] << 2 ^ (twist & 7);
    }

    inline std::function<std::uint_fast8_t(std::uint_fast8_t)> bind_rotate_twist(int rotation) {
        return std::bind(rotate_twist, rotation, std::placeholders::_1);
    }
};
