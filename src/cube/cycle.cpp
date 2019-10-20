#include <array>
#include <optional>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
using std::size_t;
using InsertionFinder::Algorithm;
using InsertionFinder::Cube;
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
    int small_cycles[7] = {0, 0, 0, 0, 0, 0, 0};
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
                ++small_cycles[orientation - 1];
            } else if ((length & 1) == 0 && orientation) {
                ++small_cycles[orientation + 1];
            } else if (length & 1) {
                parity = !parity;
                ++cycles;
                ++small_cycles[orientation + 4];
            }
        }
    }

    if (parity) {
        --cycles;
        if (small_cycles[4]) {
            --small_cycles[4];
        } else if (small_cycles[5] < small_cycles[6]) {
            --small_cycles[6];
            ++small_cycles[3];
        } else {
            --small_cycles[5];
            ++small_cycles[2];
        }
    }

    if (small_cycles[5] < small_cycles[6]) {
        small_cycles[3] += small_cycles[4] & 1;
        small_cycles[2] += (small_cycles[6] - small_cycles[5]) >> 1;
    } else {
        small_cycles[2] += small_cycles[4] & 1;
        small_cycles[3] += (small_cycles[5] - small_cycles[6]) >> 1;
    }

    int x = small_cycles[0] + small_cycles[2];
    int y = small_cycles[1] + small_cycles[3];
    cycles += (x / 3 + y / 3) << 1;
    int twists = x % 3;
    return cycles + twists + (small_cycles[2] + small_cycles[3] < twists);
}


int Cube::edge_cycles() const noexcept {
    unsigned visited_mask = 0;
    int small_cycles[3] = {0, 0, 0};
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
                    ++small_cycles[0];
                } else if (length & 1) {
                    small_cycles[2] ^= 1;
                } else {
                    ++small_cycles[1];
                }
            }
        }
    }

    small_cycles[1] += small_cycles[2];
    if (small_cycles[0] < small_cycles[1]) {
        cycles += (small_cycles[0] + small_cycles[1]) >> 1;
    } else {
        static constexpr int flip_cycles[7] = {0, 2, 3, 5, 6, 8, 9};
        cycles += small_cycles[1] + flip_cycles[(small_cycles[0] - small_cycles[1]) >> 1];
    }

    return cycles - parity;
}


std::optional<Cube> Cube::corner_cycle_cube(int index) {
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

std::optional<Cube> Cube::edge_cycle_cube(int index) {
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
                    new_cube.twist_before(Algorithm::inverse_twist[j], CubeTwist::corners);
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
                    new_cube.twist_before(Algorithm::inverse_twist[j], CubeTwist::edges);
                    new_cube.twist(j, CubeTwist::edges);
                    table[j] = new_cube.edge_cycle_index();
                }
            }
        }
    }
    return edge_cycle_transform;
}
