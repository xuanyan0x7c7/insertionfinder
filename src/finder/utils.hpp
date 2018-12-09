#pragma once

namespace InsertionFinder::Details {
    inline bool bitcount_less_than_2(std::uint32_t n) {
        return (n & (n - 1)) == 0;
    }
};
