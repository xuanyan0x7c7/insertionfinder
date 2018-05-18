#include <cstring>
#include <array>
#include <utility>
#include <fallbacks/optional.hpp>
#include <cube.hpp>
#include "utils.hpp"
using namespace std;
using namespace InsertionFinder;
using namespace Details;


namespace {
    constexpr int rotation_corner_table[4][8] = {
        {0, 3, 6, 9, 12, 15, 18, 21},
        {14, 1, 11, 22, 16, 5, 7, 20},
        {9, 0, 3, 6, 21, 12, 15, 18},
        {10, 8, 19, 23, 2, 4, 17, 13}
    };

    constexpr int rotation_edge_table[4][12] = {
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
        {9, 16, 1, 22, 13, 18, 5, 20, 10, 2, 6, 14},
        {6, 0, 2, 4, 14, 8, 10, 12, 23, 17, 19, 21},
        {23, 7, 21, 15, 17, 3, 19, 11, 1, 5, 13, 9}
    };

    constexpr int rotation_permutation[24][6] = {
        {0, 1, 2, 3, 4, 5},
        {0, 1, 4, 5, 3, 2},
        {0, 1, 3, 2, 5, 4},
        {0, 1, 5, 4, 2, 3},
        {5, 4, 2, 3, 0, 1},
        {2, 3, 4, 5, 0, 1},
        {4, 5, 3, 2, 0, 1},
        {3, 2, 5, 4, 0, 1},
        {1, 0, 2, 3, 5, 4},
        {1, 0, 4, 5, 2, 3},
        {1, 0, 3, 2, 4, 5},
        {1, 0, 5, 4, 3, 2},
        {4, 5, 2, 3, 1, 0},
        {3, 2, 4, 5, 1, 0},
        {5, 4, 3, 2, 1, 0},
        {2, 3, 5, 4, 1, 0},
        {2, 3, 1, 0, 4, 5},
        {4, 5, 1, 0, 3, 2},
        {3, 2, 1, 0, 5, 4},
        {5, 4, 1, 0, 2, 3},
        {3, 2, 0, 1, 4, 5},
        {5, 4, 0, 1, 3, 2},
        {2, 3, 0, 1, 5, 4},
        {4, 5, 0, 1, 2, 3}
    };

    constexpr bool rotation_parity_table[24] = {
        false, true, false, true,
        true, false, true, false,
        false, true, false, true,
        true, false, true, false,
        true, false, true, false,
        true, false, true, false
    };

    array<array<int, 24>, 24> generate_center_transform_table() noexcept {
        array<array<int, 24>, 24> center_transform;
        for (int i = 0; i < 24; ++i) {
            for (int j = 0; j < 24; ++j) {
                int center0 = rotation_permutation[j][rotation_permutation[i][0]];
                int center2 = rotation_permutation[j][rotation_permutation[i][2]];
                for (int k = 0; k < 24; ++k) {
                    if (
                        center0 == rotation_permutation[k][0]
                        && center2 == rotation_permutation[k][2]
                    ) {
                        center_transform[i][j] = k;
                        break;
                    }
                }
            }
        }
        return center_transform;
    }
}


const array<Cube, 24> Cube::rotation_cube = Cube::generate_rotation_cube_table();
const array<array<int, 24>, 24> Cube::center_transform = generate_center_transform_table();

array<Cube, 24> Cube::generate_rotation_cube_table() noexcept {
    array<Cube, 4> basic_rotation_cube;
    for (int i = 0; i < 4; ++i) {
        memcpy(
            basic_rotation_cube[i].corner,
            rotation_corner_table[i],
            8 * sizeof(int)
        );
        memcpy(
            basic_rotation_cube[i].edge,
            rotation_edge_table[i],
            12 * sizeof(int)
        );
    }
    array<Cube, 24> rotation_cube;
    for (int front = 0; front < 4; ++front) {
        rotation_cube[inverse_center[front | 4]].twist(basic_rotation_cube[1]);
    }
    for (int front = 0; front < 4; ++front) {
        rotation_cube[inverse_center[front | 8]].twist(basic_rotation_cube[1]);
        rotation_cube[inverse_center[front | 8]].twist(basic_rotation_cube[1]);
    }
    for (int front = 0; front < 4; ++front) {
        rotation_cube[inverse_center[front | 12]].twist(basic_rotation_cube[1]);
        rotation_cube[inverse_center[front | 12]].twist(basic_rotation_cube[1]);
        rotation_cube[inverse_center[front | 12]].twist(basic_rotation_cube[1]);
    }
    for (int front = 0; front < 4; ++front) {
        rotation_cube[inverse_center[front | 16]].twist(basic_rotation_cube[3]);
    }
    for (int front = 0; front < 4; ++front) {
        rotation_cube[inverse_center[front | 20]].twist(basic_rotation_cube[3]);
        rotation_cube[inverse_center[front | 20]].twist(basic_rotation_cube[3]);
        rotation_cube[inverse_center[front | 20]].twist(basic_rotation_cube[3]);
    }
    for (int top = 0; top < 6; ++top) {
        for (int front = 1; front < 4; ++front) {
            for (int i = 0; i < front; ++i) {
                rotation_cube[inverse_center[top << 2 | front]].twist(basic_rotation_cube[2]);
            }
        }
    }
    return rotation_cube;
}


void Cube::rotate(int rotation) {
    this->twist(Cube::rotation_cube[rotation]);
    this->_placement = Cube::center_transform[this->_placement][rotation];
}


bool Cube::placement_parity(int rotation) {
    return rotation_parity_table[rotation];
}

pair<int, Cube> Cube::best_placement() const noexcept {
    int best_rotation = 0;
    Cube best_cube = *this;
    int best_cycles = this->has_parity() + this->corner_cycles() + this->edge_cycles();
    for (int i = 1; i < 24; ++i) {
        Cube cube(*this);
        cube.twist(Cube::rotation_cube[inverse_center[i]]);
        int cycles = cube.corner_cycles() + cube.edge_cycles() + 1;
        if (rotation_parity_table[i]) {
            ++cycles;
        } else {
            cycles += cube.has_parity();
        }
        if (cycles < best_cycles) {
            best_rotation = i;
            best_cube = cube;
            best_cycles = cycles;
        }
    }
    return {best_rotation, best_cube};
}
