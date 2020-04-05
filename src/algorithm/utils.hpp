#pragma once
#include <cstdint>
#include <functional>
#include <insertionfinder/twist.hpp>

namespace InsertionFinder::Details {
    constexpr std::uint32_t twist_mask(InsertionFinder::Twist twist) {
        return 1 << twist | 1 << (24 + (twist >> 2));
    }

    inline std::function<InsertionFinder::Twist(InsertionFinder::Twist)>
    bind_rotate_twist(InsertionFinder::Rotation rotation) {
        return [rotation](InsertionFinder::Twist twist) {return twist * rotation;};
    }
};
