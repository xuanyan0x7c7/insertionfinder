#include <array>
#include <insertionfinder/algorithm.hpp>
#include <insertionfinder/cube.hpp>
using namespace std;
using namespace InsertionFinder;


bool Cube::has_parity() const noexcept {
    bool visited[8] = {false, false, false, false, false, false, false, false};
    bool parity = false;
    for (int x = 0; x < 8; ++x) {
        if (!visited[x]) {
            parity = !parity;
            int y = x;
            do {
                visited[y] = true;
                y = this->corner[y] / 3;
            } while (y != x);
        }
    }
    return parity;
}


int Cube::corner_cycles() const noexcept {
    bool visited[8] = {false, false, false, false, false, false, false, false};
    int small_cycles[7] = {0, 0, 0, 0, 0, 0, 0};
    int cycles = 0;
    bool parity = false;

    for (int x = 0; x < 8; ++x) {
        if (!visited[x]) {
            int length = -1;
            int orientation = 0;
            int y = x;
            do {
                visited[y] = true;
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
    bool visited[12] = {
        false, false, false, false, false, false,
        false, false, false, false, false, false
    };
    int small_cycles[3] = {0, 0, 0};
    int cycles = 0;
    bool parity = false;

    for (int x = 0; x < 12; ++x) {
        if (!visited[x]) {
            int length = -1;
            bool flip = false;
            int y = x;
            do {
                visited[y] = true;
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
        cycles += small_cycles[1] + flip_cycles[
            (small_cycles[0] - small_cycles[1]) >> 1
        ];
    }

    return cycles - parity;
}


Cube Cube::parity_cube(int index) {
    Cube cube;
    int w = index / 24 / 11 / 24;
    int x = index / 11 / 24 % 24;
    int y = index / 24 % 11;
    int z = index % 24;
    if (w < x / 3 && y < z >> 1) {
        cube.corner[w] = x;
        cube.corner[x / 3] = w * 3 + (24 - x) % 3;
        cube.edge[y] = z;
        cube.edge[z >> 1] = y << 1 | (z & 1);
    }
    return cube;
}

Cube Cube::corner_cycle_cube(int index) {
    Cube cube;
    int x = index / 24 / 24;
    int y = index / 24 % 24;
    int z = index % 24;
    if (x < y / 3 && x < z / 3 && y / 3 != z / 3) {
        cube.corner[x] = y;
        cube.corner[y / 3] = z;
        cube.corner[z / 3] = x * 3 + (48 - y - z) % 3;
    }
    return cube;
}

Cube Cube::edge_cycle_cube(int index) {
    Cube cube;
    int x = index / 24 / 24;
    int y = index / 24 % 24;
    int z = index % 24;
    if (x < y >> 1 && x < z >> 1 && y >> 1 != z >> 1) {
        cube.edge[x] = y;
        cube.edge[y >> 1] = z;
        cube.edge[z >> 1] = x << 1 | ((y ^ z) & 1);
    }
    return cube;
}


int Cube::parity_index() const noexcept {
    int temp = 0;
    for (int i = 0; i < 8; ++i) {
        int j = this->corner[i];
        if (j != i * 3) {
            if (j / 3 == i) {
                return -1;
            }
            temp = i * 24 + j;
            break;
        }
    }
    if (!temp) {
        return -1;
    }
    for (int i = 0; i < 12; ++i) {
        int j = this->edge[i];
        if (j != i << 1) {
            return j >> 1 == i ? -1 : temp * 11 * 24 + i * 24 + j;
        }
    }
    return -1;
}

int Cube::corner_cycle_index() const noexcept {
    for (int i = 0; i < 8; ++i) {
        int j = this->corner[i];
        if (j != i * 3) {
            int k = this->corner[j / 3];
            return k / 3 == i ? -1 : i * 24 * 24 + j * 24 + k;
        }
    }
    return -1;
}

int Cube::edge_cycle_index() const noexcept {
    for (int i = 0; i < 12; ++i) {
        int j = this->edge[i];
        if (j != i << 1) {
            int k = this->edge[j >> 1];
            return k >> 1 == i ? -1 : i * 24 * 24 + j * 24 + k;
        }
    }
    return -1;
}


void Cube::generate_parity_transform_table() noexcept {
    auto& table = Cube::parity_transform;
    for (int i = 0; i < 7 * 24 * 11 * 24; ++i) {
        Cube cube = Cube::parity_cube(i);
        if (cube.mask() == 0) {
            for (int j = 0; j < 24; ++j) {
                table[i][j] = i;
            }
        } else {
            for (int j = 0; j < 24; ++j) {
                if (j & 3) {
                    Cube new_cube = cube;
                    new_cube.twist_before(Algorithm::inverse_twist[j]);
                    new_cube.twist(j);
                    table[i][j] = new_cube.parity_index();
                } else {
                    table[i][j] = i;
                }
            }
        }
    }
}

void Cube::generate_corner_cycle_transform_table() noexcept {
   auto& table = Cube::corner_cycle_transform;
    for (int i = 0; i < 6 * 24 * 24; ++i) {
        Cube cube = Cube::corner_cycle_cube(i);
        if (cube.mask() == 0) {
            for (int j = 0; j < 24; ++j) {
                table[i][j] = i;
            }
        } else {
            for (int j = 0; j < 24; ++j) {
                if (j & 3) {
                    Cube new_cube = cube;
                    new_cube.twist_before(
                        Algorithm::inverse_twist[j],
                        CubeTwist::corners
                    );
                    new_cube.twist(j, CubeTwist::corners);
                    table[i][j] = new_cube.corner_cycle_index();
                } else {
                    table[i][j] = i;
                }
            }
        }
    }
}

void Cube::generate_edge_cycle_transform_table() noexcept {
    auto& table = Cube::edge_cycle_transform;
    for (int i = 0; i < 10 * 24 * 24; ++i) {
        Cube cube = Cube::edge_cycle_cube(i);
        if (cube.mask() == 0) {
            for (int j = 0; j < 24; ++j) {
                table[i][j] = i;
            }
        } else {
            for (int j = 0; j < 24; ++j) {
                if (j & 3) {
                    Cube new_cube = cube;
                    new_cube.twist_before(
                        Algorithm::inverse_twist[j],
                        CubeTwist::edges
                    );
                    new_cube.twist(j, CubeTwist::edges);
                    table[i][j] = new_cube.edge_cycle_index();
                } else {
                    table[i][j] = i;
                }
            }
        }
    }
}
