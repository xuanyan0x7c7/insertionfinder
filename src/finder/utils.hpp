#pragma once
#include <cstdint>

namespace InsertionFinder::Details {
    constexpr bool bitcount_less_than_2(std::uint64_t n) {
        return (n & (n - 1)) == 0;
    }
};
