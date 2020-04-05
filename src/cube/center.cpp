#include <cstddef>
#include <cstring>
#include <array>
#include <insertionfinder/cube.hpp>
using std::size_t;
using InsertionFinder::Cube;


namespace {
    constexpr unsigned rotation_corner_table[4][8] = {
        {0, 3, 6, 9, 12, 15, 18, 21},
        {14, 1, 11, 22, 16, 5, 7, 20},
        {9, 0, 3, 6, 21, 12, 15, 18},
        {10, 8, 19, 23, 2, 4, 17, 13}
    };

    constexpr unsigned rotation_edge_table[4][12] = {
        {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22},
        {9, 16, 1, 22, 13, 18, 5, 20, 10, 2, 6, 14},
        {6, 0, 2, 4, 14, 8, 10, 12, 23, 17, 19, 21},
        {23, 7, 21, 15, 17, 3, 19, 11, 1, 5, 13, 9}
    };
}


std::array<Cube, 24> Cube::generate_rotation_cube_table() noexcept {
    std::array<Cube, 24> rotation_cube;
    Cube basic_rotation_cube[4];
    for (size_t i = 0; i < 4; ++i) {
        std::memcpy(basic_rotation_cube[i].corner, rotation_corner_table[i], 8 * sizeof(unsigned));
        std::memcpy(basic_rotation_cube[i].edge, rotation_edge_table[i], 12 * sizeof(unsigned));
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

const std::array<Cube, 24> Cube::rotation_cube = Cube::generate_rotation_cube_table();


Cube Cube::best_placement() const noexcept {
    Cube original_cube = *this;
    original_cube.rotate(this->_placement.inverse());
    int best_cycles = original_cube.corner_cycles() + original_cube.edge_cycles() + original_cube.has_parity();
    if (best_cycles <= 4) {
        return original_cube;
    }

    Cube best_cube = original_cube;
    for (size_t rotation: {
        2, 8, 10,
        5, 7, 13, 15, 17, 19, 21, 23,
        1, 3, 4, 12, 16, 20,
        6, 9, 11, 14, 18, 22
    }) {
        Cube cube = original_cube * Cube::rotation_cube[rotation];
        int cycles = cube.corner_cycles() + cube.edge_cycles();
        if (Cube::center_cycles[rotation] <= 1) {
            cycles += cube.has_parity();
        }
        if (cycles <= 4) {
            return cube;
        }
        if (cycles + Cube::center_cycles[rotation] < best_cycles) {
            best_cube = cube;
            best_cycles = cycles + Cube::center_cycles[rotation];
        }
    }
    return best_cube;
}
