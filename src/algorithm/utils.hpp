#include <cstdint>
#include <array>

namespace InsertionFinder::Details {
    inline std::uint32_t twist_mask(int twist) {
        return 1 << twist | 1 << (24 + (twist >> 2));
    }

    inline int transform_twist(const int* transform, int twist) {
        return transform[twist >> 3] << 2 ^ (twist & 7);
    }

    constexpr int rotation_permutation[24][3] = {
        {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
        {5, 2, 0}, {2, 4, 0}, {4, 3, 0}, {3, 5, 0},
        {1, 2, 5}, {1, 4, 2}, {1, 3, 4}, {1, 5, 3},
        {4, 2, 1}, {3, 4, 1}, {5, 3, 1}, {2, 5, 1},
        {2, 1, 4}, {4, 1, 3}, {3, 1, 5}, {5, 1, 2},
        {3, 0, 4}, {5, 0, 3}, {2, 0, 5}, {4, 0, 2}
    };
};
