#include <stdbool.h>
#include <string.h>
#include "cube.h"
#include "common.h"


static Cube ParityCube(int index);
static Cube Corner3CycleCube(int index);
static Cube Edge3CycleCube(int index);


bool CubeHasParity(const Cube* cube) {
    bool visited[] = {false, false, false, false, false, false, false, false};
    const int* corner = cube->corner;
    bool parity = false;
    for (int x = 0; x < 8; ++x) {
        if (!visited[x]) {
            parity = !parity;
            int y = x;
            do {
                visited[y] = true;
                y = corner[y] / 3;
            } while (y != x);
        }
    }
    return parity;
}


int CubeCornerCycles(const Cube* cube) {
    bool visited[] = {false, false, false, false, false, false, false, false};
    const int* corner = cube->corner;
    int small_cycles[] = {0, 0, 0, 0, 0, 0, 0};
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
                orientation += corner[y];
                y = corner[y] / 3;
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


int CubeEdgeCycles(const Cube* cube) {
    bool visited[] = {
        false, false, false, false, false, false,
        false, false, false, false, false, false
    };
    const int* edge = cube->edge;
    int small_cycles[] = {0, 0, 0};
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
                flip ^= edge[y] & 1;
                y = edge[y] >> 1;
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
        static const int flip_cycles[] = {0, 2, 3, 5, 6, 8, 9};
        cycles += small_cycles[1] + flip_cycles[
            (small_cycles[0] - small_cycles[1]) >> 1
        ];
    }

    return cycles - parity;
}


int CubeParityIndex(const Cube* cube) {
    const int* corner = cube->corner;
    const int* edge = cube->edge;
    int temp = 0;
    for (int i = 0; i < 8; ++i) {
        int j = corner[i];
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
        int j = edge[i];
        if (j != i << 1) {
            return j >> 1 == i ? -1 : temp * 11 * 24 + i * 24 + j;
        }
    }
    return -1;
}

int CubeCorner3CycleIndex(const Cube* cube) {
    const int* corner = cube->corner;
    for (int i = 0; i < 8; ++i) {
        int j = corner[i];
        if (j != i * 3) {
            int k = corner[j / 3];
            return k / 3 == i ? -1 : i * 24 * 24 + j * 24 + k;
        }
    }
    return -1;
}

int CubeEdge3CycleIndex(const Cube* cube) {
    const int* edge = cube->edge;
    for (int i = 0; i < 12; ++i) {
        int j = edge[i];
        if (j != i << 1) {
            int k = edge[j >> 1];
            return k >> 1 == i ? -1 : i * 24 * 24 + j * 24 + k;
        }
    }
    return -1;
}

int CubeParityNextIndex(int index, int move) {
    return parity_transform_table[index][move];
}

int CubeCornerNext3CycleIndex(int index, int move) {
    return corner_cycle_transform_table[index][move];
}

int CubeEdgeNext3CycleIndex(int index, int move) {
    return edge_cycle_transform_table[index][move];
}


void GenerateParityTable(int table[][24]) {
    for (int i = 0; i < 7 * 24 * 11 * 24; ++i) {
        Cube cube = ParityCube(i);
        if (CubeMask(&cube) == 0) {
            for (int j = 0; j < 24; ++j) {
                table[i][j] = i;
            }
        } else {
            for (int j = 0; j < 24; ++j) {
                if (j & 3) {
                    Cube new_cube;
                    memcpy(
                        &new_cube,
                        &one_move_cube[inverse_move_table[j]],
                        sizeof(Cube)
                    );
                    CubeTwistCube(&new_cube, &cube, true, true);
                    CubeTwistMove(&new_cube, j, true, true);
                    table[i][j] = CubeParityIndex(&new_cube);
                } else {
                    table[i][j] = i;
                }
            }
        }
    }
}

void GenerateCornerCycleTable(int table[][24]) {
    for (int i = 0; i < 6 * 24 * 24; ++i) {
        Cube cube = Corner3CycleCube(i);
        if (CubeMask(&cube) == 0) {
            for (int j = 0; j < 24; ++j) {
                table[i][j] = i;
            }
        } else {
            for (int j = 0; j < 24; ++j) {
                if (j & 3) {
                    Cube new_cube;
                    memcpy(
                        new_cube.corner,
                        one_move_cube[inverse_move_table[j]].corner,
                        8 * sizeof(int)
                    );
                    CubeTwistCube(&new_cube, &cube, true, false);
                    CubeTwistMove(&new_cube, j, true, false);
                    table[i][j] = CubeCorner3CycleIndex(&new_cube);
                } else {
                    table[i][j] = i;
                }
            }
        }
    }
}

void GenerateEdgeCycleTable(int table[][24]) {
    for (int i = 0; i < 10 * 24 * 24; ++i) {
        Cube cube = Edge3CycleCube(i);
        if (CubeMask(&cube) == 0) {
            for (int j = 0; j < 24; ++j) {
                table[i][j] = i;
            }
        } else {
            for (int j = 0; j < 24; ++j) {
                if (j & 3) {
                    Cube new_cube;
                    memcpy(
                        new_cube.edge,
                        one_move_cube[inverse_move_table[j]].edge,
                        12 * sizeof(int)
                    );
                    CubeTwistCube(&new_cube, &cube, false, true);
                    CubeTwistMove(&new_cube, j, false, true);
                    table[i][j] = CubeEdge3CycleIndex(&new_cube);
                } else {
                    table[i][j] = i;
                }
            }
        }
    }
}


Cube ParityCube(int index) {
    Cube cube = identity_cube;
    int w = index / 24 / 11 / 24;
    int x = (index / 11 / 24) % 24;
    int y = (index / 24) % 11;
    int z = index % 24;
    if (w != x / 3 && y != z >> 1) {
        cube.corner[w] = x;
        cube.corner[x / 3] = w * 3 + (24 - x) % 3;
        cube.edge[y] = z;
        cube.edge[z >> 1] = (y << 1) | (z & 1);
    }
    return cube;
}

Cube Corner3CycleCube(int index) {
    Cube cube = identity_cube;
    int x = index / 24 / 24;
    int y = (index / 24) % 24;
    int z = index % 24;
    if (x != y / 3 && x != z / 3 && y / 3 != z / 3) {
        cube.corner[x] = y;
        cube.corner[y / 3] = z;
        cube.corner[z / 3] = x * 3 + (48 - y - z) % 3;
    }
    return cube;
}

Cube Edge3CycleCube(int index) {
    Cube cube = identity_cube;
    int x = index / 24 / 24;
    int y = (index / 24) % 24;
    int z = index % 24;
    if (x != y >> 1 && x != z >> 1 && y >> 1 != z >> 1) {
        cube.edge[x] = y;
        cube.edge[y >> 1] = z;
        cube.edge[z >> 1] = (x << 1) | ((y + z) & 1);
    }
    return cube;
}
