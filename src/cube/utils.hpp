#pragma once

namespace InsertionFinder::Details {
    constexpr int inverse_center[24] = {
        0, 3, 2, 1,
        12, 23, 6, 17,
        8, 9, 10, 11,
        4, 19, 14, 21,
        20, 7, 18, 13,
        16, 15, 22, 5
    };

    constexpr int center_cycles[24] = {
        0, 2, 1, 2,
        2, 1, 3, 1,
        1, 3, 1, 3,
        2, 1, 3, 1,
        2, 1, 3, 1,
        2, 1, 3, 1
    };
};
