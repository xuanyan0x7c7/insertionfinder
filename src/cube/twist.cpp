#include <cstring>
#include <array>
#include <optional>
#include <algorithm.hpp>
#include <cube.hpp>
using namespace std;
using namespace InsertionFinder;


namespace {
    constexpr int corner_twist_table[24][8] = {
        {0, 3, 6, 9, 12, 15, 18, 21},
        {9, 0, 3, 6, 12, 15, 18, 21},
        {6, 9, 0, 3, 12, 15, 18, 21},
        {3, 6, 9, 0, 12, 15, 18, 21},
        {0, 3, 6, 9, 12, 15, 18, 21},
        {0, 3, 6, 9, 15, 18, 21, 12},
        {0, 3, 6, 9, 18, 21, 12, 15},
        {0, 3, 6, 9, 21, 12, 15, 18},
        {0, 3, 6, 9, 12, 15, 18, 21},
        {0, 3, 11, 22, 12, 15, 7, 20},
        {0, 3, 21, 18, 12, 15, 9, 6},
        {0, 3, 20, 7, 12, 15, 22, 11},
        {0, 3, 6, 9, 12, 15, 18, 21},
        {5, 16, 6, 9, 1, 14, 18, 21},
        {15, 12, 6, 9, 3, 0, 18, 21},
        {14, 1, 6, 9, 16, 5, 18, 21},
        {0, 3, 6, 9, 12, 15, 18, 21},
        {0, 8, 19, 9, 12, 4, 17, 21},
        {0, 18, 15, 9, 12, 6, 3, 21},
        {0, 17, 4, 9, 12, 19, 8, 21},
        {0, 3, 6, 9, 12, 15, 18, 21},
        {13, 3, 6, 2, 23, 15, 18, 10},
        {21, 3, 6, 12, 9, 15, 18, 0},
        {10, 3, 6, 23, 2, 15, 18, 13}
    };

    constexpr int edge_twist_table[24][12] = {
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
        {6, 0, 2, 4, 8, 10, 12, 14, 16, 18, 20, 22},
        {4, 6, 0, 2, 8, 10, 12, 14, 16, 18, 20, 22},
        {2, 4, 6, 0, 8, 10, 12, 14, 16, 18, 20, 22},
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
        {0, 2, 4, 6, 10, 12, 14, 8, 16, 18, 20, 22},
        {0, 2, 4, 6, 12, 14, 8, 10, 16, 18, 20, 22},
        {0, 2, 4, 6, 14, 8, 10, 12, 16, 18, 20, 22},
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
        {0, 2, 4, 22, 8, 10, 12, 20, 16, 18, 6, 14},
        {0, 2, 4, 14, 8, 10, 12, 6, 16, 18, 22, 20},
        {0, 2, 4, 20, 8, 10, 12, 22, 16, 18, 14, 6},
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
        {0, 18, 4, 6, 8, 16, 12, 14, 2, 10, 20, 22},
        {0, 10, 4, 6, 8, 2, 12, 14, 18, 16, 20, 22},
        {0, 16, 4, 6, 8, 18, 12, 14, 10, 2, 20, 22},
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
        {0, 2, 21, 6, 8, 10, 19, 14, 16, 5, 13, 22},
        {0, 2, 12, 6, 8, 10, 4, 14, 16, 20, 18, 22},
        {0, 2, 19, 6, 8, 10, 21, 14, 16, 13, 5, 22},
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
        {17, 2, 4, 6, 23, 10, 12, 14, 9, 18, 20, 1},
        {8, 2, 4, 6, 0, 10, 12, 14, 22, 18, 20, 16},
        {23, 2, 4, 6, 17, 10, 12, 14, 1, 18, 20, 9}
    };
};


array<Cube, 24> Cube::generate_twist_cube_table() noexcept {
    array<Cube, 24> twist_cube;
    for (int i = 0; i < 24; ++i) {
        memcpy(twist_cube[i].corner, corner_twist_table[i], 8 * sizeof(int));
        memcpy(twist_cube[i].edge, edge_twist_table[i], 12 * sizeof(int));
    }
    return twist_cube;
}


void Cube::twist(int twist, uint8_t flags) {
    this->twist(this->twist_cube[twist], flags);
}

void Cube::twist(const Algorithm& algorithm, uint8_t flags) noexcept {
    this->twist(algorithm, 0, algorithm.length(), flags);
}

void Cube::twist(
    const Algorithm& algorithm,
    size_t begin, size_t end,
    uint8_t flags
) noexcept {
    if (flags & CubeTwist::reversed) {
        for (size_t i = end; i > begin; --i) {
            this->twist(
                Algorithm::inverse_twist[algorithm[i]],
                flags
            );
        }
    } else {
        for (size_t i = begin; i < end; ++i) {
            this->twist(algorithm[i], flags);
        }
    }
}

void Cube::twist(const Cube& cube, uint8_t flags) noexcept {
    if (flags & CubeTwist::corners) {
        int* begin = this->corner;
        int* end = begin + 8;
        for (int* item = begin; item < end; ++item) {
            int transform = cube.corner[*item / 3];
            *item = transform - transform % 3 + (*item + transform) % 3;
        }
    }
    if (flags & CubeTwist::edges) {
        int* begin = this->edge;
        int* end = begin + 12;
        for (int* item = begin; item < end; ++item) {
            *item = cube.edge[*item >> 1] ^ (*item & 1);
        }
    }
}


void Cube::twist_before(int twist, uint8_t flags) {
    this->twist_before(this->twist_cube[twist], flags);
}

void Cube::twist_before(const Cube& cube, uint8_t flags) noexcept {
    if (flags & CubeTwist::corners) {
        int corner[8];
        for (int i = 0; i < 8; ++i) {
            int item = cube.corner[i];
            int transform = this->corner[item / 3];
            corner[i] = transform - transform % 3 + (item + transform) % 3;
        }
        memcpy(this->corner, corner, 8 * sizeof(int));
    }
    if (flags & CubeTwist::edges) {
        int edge[12];
        for (int i = 0; i < 12; ++i) {
            int item = cube.edge[i];
            edge[i] = this->edge[item >> 1] ^ (item & 1);
        }
        memcpy(this->edge, edge, 12 * sizeof(int));
    }
}


optional<Cube> Cube::twist_effectively(
    const Cube& cube, uint8_t flags
) const noexcept {
    Cube result = *this;
    if (flags & CubeTwist::corners) {
        for (int i = 0; i < 8; ++i) {
            int item = this->corner[i];
            int transform = cube.corner[item / 3];
            int new_corner = transform - transform % 3 + (item + transform) % 3;
            if (new_corner / 3 == i && new_corner % 3 && new_corner != item) {
                return nullopt;
            }
            result.corner[i] = new_corner;
        }
    }
    if (flags & CubeTwist::edges) {
        for (int i = 0; i < 12; ++i) {
            int item = this->edge[i];
            int new_edge = cube.edge[item >> 1] ^ (item & 1);
            if (new_edge == (i << 1 | 1) && new_edge != item) {
                return nullopt;
            }
            result.edge[i] = new_edge;
        }
    }
    return result;
}
