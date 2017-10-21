#include <cstdint>

namespace InsertionFinder::Details {
    inline std::uint32_t twist_mask(int twist) {
        return 1 << twist | 1 << (24 + (twist >> 2));
    }
};
