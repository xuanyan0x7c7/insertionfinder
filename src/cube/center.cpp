#include <cstring>
#include <array>
#include <insertionfinder/cube.hpp>
using namespace std;
using namespace InsertionFinder;


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

    array<array<int, 24>, 24> generate_center_transform_table() noexcept {
        array<array<int, 24>, 24> center_transform;
        for (size_t i = 0; i < 24; ++i) {
            for (size_t j = 0; j < 24; ++j) {
                int center0 = rotation_permutation[j][rotation_permutation[i][0]];
                int center2 = rotation_permutation[j][rotation_permutation[i][2]];
                for (int k = 0; k < 24; ++k) {
                    if (center0 == rotation_permutation[k][0] && center2 == rotation_permutation[k][2]) {
                        center_transform[i][j] = k;
                        break;
                    }
                }
            }
        }
        return center_transform;
    }
}


const array<array<int, 24>, 24> Cube::center_transform = generate_center_transform_table();

array<Cube, 24> Cube::generate_rotation_cube_table() noexcept {
    array<Cube, 24> rotation_cube;
    array<Cube, 4> basic_rotation_cube;
    for (size_t i = 0; i < 4; ++i) {
        memcpy(basic_rotation_cube[i].corner, rotation_corner_table[i], 8 * sizeof(int));
        memcpy(basic_rotation_cube[i].edge, rotation_edge_table[i], 12 * sizeof(int));
    }
    for (size_t front = 0; front < 4; ++front) {
        rotation_cube[front | 4].twist(basic_rotation_cube[1]);
        rotation_cube[front | 8].twist(basic_rotation_cube[1]);
        rotation_cube[front | 8].twist(basic_rotation_cube[1]);
        rotation_cube[front | 12].twist(basic_rotation_cube[1]);
        rotation_cube[front | 12].twist(basic_rotation_cube[1]);
        rotation_cube[front | 12].twist(basic_rotation_cube[1]);
        rotation_cube[front | 16].twist(basic_rotation_cube[3]);
        rotation_cube[front | 20].twist(basic_rotation_cube[3]);
        rotation_cube[front | 20].twist(basic_rotation_cube[3]);
        rotation_cube[front | 20].twist(basic_rotation_cube[3]);
    }
    for (size_t top = 0; top < 6; ++top) {
        for (size_t front = 1; front < 4; ++front) {
            for (size_t i = 0; i < front; ++i) {
                rotation_cube[top << 2 | front].twist(basic_rotation_cube[2]);
            }
        }
    }
    for (int placement = 0; placement < 24; ++placement) {
        rotation_cube[placement]._placement = placement;
    }
    return rotation_cube;
}

const array<Cube, 24> Cube::rotation_cube = Cube::generate_rotation_cube_table();


Cube Cube::best_placement() const noexcept {
    Cube original_cube = *this;
    original_cube.rotate(Cube::inverse_center[original_cube._placement]);
    int best_cycles = original_cube.corner_cycles() + original_cube.edge_cycles() + original_cube.has_parity();
    if (best_cycles <= 4) {
        return original_cube;
    }

    Cube best_cube = original_cube;
    for (int index: {
        2, 8, 10,
        5, 7, 13, 15, 17, 19, 21, 23,
        1, 3, 4, 12, 16, 20,
        6, 9, 11, 14, 18, 22
    }) {
        Cube cube = original_cube;
        cube.rotate(index);
        int cycles = cube.corner_cycles() + cube.edge_cycles();
        if (Cube::center_cycles[index] <= 1) {
            cycles += cube.has_parity();
        }
        if (cycles + Cube::center_cycles[index] < best_cycles) {
            best_cube = cube;
            best_cycles = cycles + Cube::center_cycles[index];
            if (cycles <= 4) {
                return best_cube;
            }
        }
    }
    return best_cube;
}
