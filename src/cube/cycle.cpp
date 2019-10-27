#include <array>
#include <optional>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
#include <insertionfinder/twist.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Cube;
using InsertionFinder::Twist;
namespace CubeTwist = InsertionFinder::CubeTwist;


bool Cube::has_parity() const noexcept {
    unsigned visited_mask = 0;
    bool parity = false;
    for (unsigned x = 0; x < 8; ++x) {
        if ((visited_mask & (1 << x)) == 0) {
            parity = !parity;
            unsigned y = x;
            do {
                visited_mask |= 1 << y;
                y = this->corner[y] / 3;
            } while (y != x);
        }
    }
    return parity;
}


int Cube::corner_cycles() const noexcept {
    unsigned visited_mask = 0;
    int self_orientation[3] = {0, 0, 0};
    int even_permutation[3] = {0, 0, 0};
    int odd_permutation[3] = {0, 0, 0};
    int cycles = 0;
    bool parity = false;

    for (unsigned x = 0; x < 8; ++x) {
        if ((visited_mask & (1 << x)) == 0) {
            int length = -1;
            unsigned orientation = 0;
            unsigned y = x;
            do {
                visited_mask |= 1 << y;
                ++length;
                orientation += this->corner[y];
                y = this->corner[y] / 3;
            } while (y != x);
            cycles += length >> 1;
            orientation %= 3;
            if (length == 0 && orientation) {
                ++self_orientation[orientation];
            } else if ((length & 1) == 0 && orientation) {
                ++even_permutation[orientation];
            } else if (length & 1) {
                parity = !parity;
                ++cycles;
                ++odd_permutation[orientation];
            }
        }
    }

    if (parity) {
        --cycles;
        if (odd_permutation[0]) {
            --odd_permutation[0];
        } else if (odd_permutation[1] < odd_permutation[2]) {
            --odd_permutation[2];
            ++even_permutation[2];
        } else {
            --odd_permutation[1];
            ++even_permutation[1];
        }
    }

    if (odd_permutation[1] < odd_permutation[2]) {
        even_permutation[2] += odd_permutation[0] & 1;
        even_permutation[1] += (odd_permutation[2] - odd_permutation[1]) >> 1;
    } else {
        even_permutation[1] += odd_permutation[0] & 1;
        even_permutation[2] += (odd_permutation[1] - odd_permutation[2]) >> 1;
    }

    int x = self_orientation[1] + even_permutation[1];
    int y = self_orientation[2] + even_permutation[2];
    cycles += (x / 3 + y / 3) << 1;
    int twists = x % 3;
    return cycles + twists + (even_permutation[1] + even_permutation[2] < twists);
}


int Cube::edge_cycles() const noexcept {
    unsigned visited_mask = 0;
    int self_flip = 0;
    int even_flip = 0;
    int odd_flip = 0;
    int cycles = 0;
    bool parity = false;

    for (unsigned x = 0; x < 12; ++x) {
        if ((visited_mask & (1 << x)) == 0) {
            int length = -1;
            bool flip = false;
            unsigned y = x;
            do {
                visited_mask |= 1 << y;
                ++length;
                flip ^= this->edge[y] & 1;
                y = this->edge[y] >> 1;
            } while (y != x);
            cycles += length >> 1;
            if (length & 1) {
                parity = !parity;
                ++cycles;
            }
            if (flip) {
                if (length == 0) {
                    ++self_flip;
                } else if (length & 1) {
                    odd_flip ^= 1;
                } else {
                    ++even_flip;
                }
            }
        }
    }

    even_flip += odd_flip;
    if (self_flip < even_flip) {
        cycles += (self_flip + even_flip) >> 1;
    } else {
        static constexpr int flip_cycles[7] = {0, 2, 3, 5, 6, 8, 9};
        cycles += even_flip + flip_cycles[(self_flip - even_flip) >> 1];
    }

    return cycles - parity;
}


std::optional<Cube> Cube::corner_cycle_cube(unsigned index) {
    unsigned x = index / 24 / 24;
    unsigned y = index / 24 % 24;
    unsigned z = index % 24;
    if (x < y / 3 && x < z / 3 && y / 3 != z / 3) {
        Cube cube;
        cube.corner[x] = y;
        cube.corner[y / 3] = z;
        cube.corner[z / 3] = x * 3 + (48 - y - z) % 3;
        return cube;
    } else {
        return std::nullopt;
    }
}

std::optional<Cube> Cube::edge_cycle_cube(unsigned index) {
    unsigned x = index / 24 / 24;
    unsigned y = index / 24 % 24;
    unsigned z = index % 24;
    if (x < y >> 1 && x < z >> 1 && y >> 1 != z >> 1) {
        Cube cube;
        cube.edge[x] = y;
        cube.edge[y >> 1] = z;
        cube.edge[z >> 1] = x << 1 | ((y ^ z) & 1);
        return cube;
    } else {
        return std::nullopt;
    }
}


int Cube::corner_cycle_index() const noexcept {
    for (unsigned i = 0; i < 8; ++i) {
        unsigned j = this->corner[i];
        if (i != j / 3) {
            unsigned k = this->corner[j / 3];
            return k / 3 == i ? -1 : i * 24 * 24 + j * 24 + k;
        }
    }
    return -1;
}

int Cube::edge_cycle_index() const noexcept {
    for (unsigned i = 0; i < 12; ++i) {
        unsigned j = this->edge[i];
        if (i != j >> 1) {
            unsigned k = this->edge[j >> 1];
            return k >> 1 == i ? -1 : i * 24 * 24 + j * 24 + k;
        }
    }
    return -1;
}


const std::vector<std::array<int, 24>> Cube::corner_cycle_transform = generate_corner_cycle_transform_table();
const std::vector<std::array<int, 24>> Cube::edge_cycle_transform = generate_edge_cycle_transform_table();

std::vector<std::array<int, 24>> Cube::generate_corner_cycle_transform_table() {
    std::vector<std::array<int, 24>> corner_cycle_transform(6 * 24 * 24);
    for (size_t i = 0; i < 6 * 24 * 24; ++i) {
        auto& table = corner_cycle_transform[i];
        if (auto cube = Cube::corner_cycle_cube(i)) {
            for (size_t j = 0; j < 24; ++j) {
                if (j & 3) {
                    Cube new_cube = *cube;
                    new_cube.twist_before(Twist::inverse(j), CubeTwist::corners);
                    new_cube.twist(j, CubeTwist::corners);
                    table[j] = new_cube.corner_cycle_index();
                }
            }
        }
    }
    return corner_cycle_transform;
}

std::vector<std::array<int, 24>> Cube::generate_edge_cycle_transform_table() {
    std::vector<std::array<int, 24>> edge_cycle_transform(10 * 24 * 24);
    for (size_t i = 0; i < 10 * 24 * 24; ++i) {
        auto& table = edge_cycle_transform[i];
        if (auto cube = Cube::edge_cycle_cube(i)) {
            for (size_t j = 0; j < 24; ++j) {
                if (j & 3) {
                    Cube new_cube = *cube;
                    new_cube.twist_before(Twist::inverse(j), CubeTwist::edges);
                    new_cube.twist(j, CubeTwist::edges);
                    table[j] = new_cube.edge_cycle_index();
                }
            }
        }
    }
    return edge_cycle_transform;
}
