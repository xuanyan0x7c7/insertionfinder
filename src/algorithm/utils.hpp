#pragma once
#include <cstdint>
#include <functional>
#include <insertionfinder/twist.hpp>

namespace InsertionFinder::Details {
    constexpr std::uint32_t twist_mask(Twist twist) {
        return 1 << twist | 1 << (24 + (twist >> 2));
    }

    inline std::function<Twist(Twist)> bind_rotate_twist(Rotation rotation) {
        return [rotation](Twist twist) {return twist * rotation;};
    }
};
