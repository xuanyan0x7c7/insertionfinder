#include <stdint.h>
#include <stdlib.h>
#include "formula.h"
#include "cube.h"


static const int corner_twist_table[][8] = {
    {},
    {9, 0, 3, 6, 12, 15, 18, 21},
    {6, 9, 0, 3, 12, 15, 18, 21},
    {3, 6, 9, 0, 12, 15, 18, 21},
    {},
    {0, 3, 6, 9, 15, 18, 21, 12},
    {0, 3, 6, 9, 18, 21, 12, 15},
    {0, 3, 6, 9, 21, 12, 15, 18},
    {},
    {0, 3, 11, 22, 12, 15, 7, 20},
    {0, 3, 21, 18, 12, 15, 9, 6},
    {0, 3, 20, 7, 12, 15, 22, 11},
    {},
    {5, 16, 6, 9, 1, 14, 18, 21},
    {15, 12, 6, 9, 3, 0, 18, 21},
    {14, 1, 6, 9, 16, 5, 18, 21},
    {},
    {0, 8, 19, 9, 12, 4, 17, 21},
    {0, 18, 15, 9, 12, 6, 3, 21},
    {0, 17, 4, 9, 12, 19, 8, 21},
    {},
    {13, 3, 6, 2, 23, 15, 18, 10},
    {21, 3, 6, 12, 9, 15, 18, 0},
    {10, 3, 6, 23, 2, 15, 18, 13}
};

static const int edge_twist_table[][12] = {
    {},
    {6, 0, 2, 4, 8, 10, 12, 14, 16, 18, 20, 22},
    {4, 6, 0, 2, 8, 10, 12, 14, 16, 18, 20, 22},
    {2, 4, 6, 0, 8, 10, 12, 14, 16, 18, 20, 22},
    {},
    {0, 2, 4, 6, 10, 12, 14, 8, 16, 18, 20, 22},
    {0, 2, 4, 6, 12, 14, 8, 10, 16, 18, 20, 22},
    {0, 2, 4, 6, 14, 8, 10, 12, 16, 18, 20, 22},
    {},
    {0, 2, 4, 22, 8, 10, 12, 20, 16, 18, 6, 14},
    {0, 2, 4, 14, 8, 10, 12, 6, 16, 18, 22, 20},
    {0, 2, 4, 20, 8, 10, 12, 22, 16, 18, 14, 6},
    {},
    {0, 18, 4, 6, 8, 16, 12, 14, 2, 10, 20, 22},
    {0, 10, 4, 6, 8, 2, 12, 14, 18, 16, 20, 22},
    {0, 16, 4, 6, 8, 18, 12, 14, 10, 2, 20, 22},
    {},
    {0, 2, 21, 6, 8, 10, 19, 14, 16, 5, 13, 22},
    {0, 2, 12, 6, 8, 10, 4, 14, 16, 20, 18, 22},
    {0, 2, 19, 6, 8, 10, 21, 14, 16, 13, 5, 22},
    {},
    {17, 2, 4, 6, 23, 10, 12, 14, 9, 18, 20, 1},
    {8, 2, 4, 6, 0, 10, 12, 14, 22, 18, 20, 16},
    {23, 2, 4, 6, 17, 10, 12, 14, 1, 18, 20, 9}
};

static Cube Corner3CycleCube(int index);
static Cube Edge3CycleCube(int index);


Cube* CubeConstruct(Cube* cube, const Formula* formula) {
    if (!cube) {
        cube = (Cube*)malloc(sizeof(Cube));
    }
    for (int i = 0; i < 8; ++i) {
        cube->corner[i] = i * 3;
    }
    for (int i = 0; i < 12; ++i) {
        cube->edge[i] = i << 1;
    }
    CubeTwistFormula(cube, formula, true, true, false);
    return cube;
}


void CubeSave(const Cube* cube, FILE* stream) {
    int8_t corner[8], edge[12];
    for (int i = 0; i < 8; ++i) {
        corner[i] = cube->corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        edge[i] = cube->edge[i];
    }
    fwrite(corner, sizeof(int8_t), 8, stream);
    fwrite(edge, sizeof(int8_t), 12, stream);
}

Cube* CubeLoad(Cube* cube, FILE* stream) {
    if (!cube) {
        cube = (Cube*)malloc(sizeof(Cube));
    }
    int8_t corner[8], edge[12];
    fread(corner, sizeof(int8_t), 8, stream);
    fread(edge, sizeof(int8_t), 12, stream);
    for (int i = 0; i < 8; ++i) {
        cube->corner[i] = corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        cube->edge[i] = edge[i];
    }
    return cube;
}


void CubeTwist(Cube* cube, int move) {
    CubeTwistCorner(cube, move);
    CubeTwistEdge(cube, move);
}

void CubeTwistCorner(Cube* cube, int move) {
    const int* table = corner_twist_table[move];
    for (int i = 0; i < 8; ++i) {
        int* item = &(cube->corner[i]);
        int transform = table[*item / 3];
        *item = transform - transform % 3 + (*item + transform) % 3;
    }
}

void CubeTwistEdge(Cube* cube, int move) {
    const int* table = edge_twist_table[move];
    for (int i = 0; i < 12; ++i) {
        int* item = &(cube->edge[i]);
        *item = table[*item >> 1] ^ (*item & 1);
    }
}

void CubeTwistFormula(
    Cube* cube,
    const Formula* formula,
    bool twist_corners, bool twist_edges,
    bool reversed
) {
    CubeRangeTwistFormula(
        cube,
        formula,
        0, formula->length,
        twist_corners, twist_edges,
        reversed
    );
}

void CubeRangeTwistFormula(
    Cube* cube,
    const Formula* formula,
    size_t begin, size_t end,
    bool twist_corners, bool twist_edges,
    bool reversed
) {
    if (reversed) {
        for (size_t i = end; i > begin; --i) {
            int move = inverse_move_table[formula->move[i - 1]];
            if (twist_corners) {
                CubeTwistCorner(cube, move);
            }
            if (twist_edges) {
                CubeTwistEdge(cube, move);
            }
        }
    } else {
        for (size_t i = begin; i < end; ++i) {
            int move = formula->move[i];
            if (twist_corners) {
                CubeTwistCorner(cube, move);
            }
            if (twist_edges) {
                CubeTwistEdge(cube, move);
            }
        }
    }
}

void CubeTwistCube(Cube* cube, const Cube* state) {
    for (int i = 0; i < 8; ++i) {
        int* item = &(cube->corner[i]);
        int transform = state->corner[*item / 3];
        *item = transform - transform % 3 + (*item + transform) % 3;
    }
    for (int i = 0; i < 12; ++i) {
        int* item = &(cube->edge[i]);
        *item = state->edge[*item >> 1] ^ (*item & 1);
    }
}


Cube* CubeInverseState(Cube* cube, const Cube* state) {
    if (!cube) {
        cube = (Cube*)malloc(sizeof(Cube));
    }
    for (int i = 0; i < 8; ++i) {
        cube->corner[state->corner[i] / 3] = i * 3 + (3 - state->corner[i]) % 3;
    }
    for (int i = 0; i < 12; ++i) {
        cube->edge[state->edge[i] >> 1] = (i << 1) | (state->edge[i] & 1);
    }
    return cube;
}


unsigned CubeMask(const Cube* cube) {
    unsigned mask = 0;
    for (int i = 0; i < 8; ++i) {
        mask <<= 1;
        if (cube->corner[i] != i * 3) {
            mask |= 1;
        }
    }
    for (int i = 0; i < 12; ++i) {
        mask <<= 1;
        if (cube->edge[i] != i << 1) {
            mask |= 1;
        }
    }
    return mask;
}


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
                ++cycles;
                ++small_cycles[orientation + 4];
            }
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

    return cycles;
}


int CubeCorner3CycleIndex(const Cube* cube) {
    int x;
    for (int i = 0; i < 8; ++i) {
        if (cube->corner[i] != i * 3) {
            x = i;
            break;
        }
    }
    int y = cube->corner[x];
    int z = cube->corner[y / 3];
    return x * 24 * 24 + y * 24 + z;
}

int CubeEdge3CycleIndex(const Cube* cube) {
    int x;
    for (int i = 0; i < 12; ++i) {
        if (cube->edge[i] != i << 1) {
            x = i;
            break;
        }
    }
    int y = cube->edge[x];
    int z = cube->edge[y >> 1];
    return x * 24 * 24 + y * 24 + z;
}


Cube Corner3CycleCube(int index) {
    Cube cube;
    int x = index / 24 / 24;
    int y = (index / 24) % 24;
    int z = index % 24;
    for (int i = 0; i < 8; ++i) {
        cube.corner[i] = i * 3;
    }
    for (int i = 0; i < 12; ++i) {
        cube.edge[i] = i << 1;
    }
    cube.corner[x] = y;
    cube.corner[y / 3] = z;
    cube.corner[z / 3] = x * 3 + (48 - y - z) % 3;
    return cube;
}

Cube Edge3CycleCube(int index) {
    Cube cube;
    int x = index / 24 / 24;
    int y = (index / 24) % 24;
    int z = index % 24;
    for (int i = 0; i < 8; ++i) {
        cube.corner[i] = i * 3;
    }
    for (int i = 0; i < 12; ++i) {
        cube.edge[i] = i << 1;
    }
    cube.edge[x] = y;
    cube.edge[y >> 1] = z;
    cube.edge[z >> 1] = (x << 1) | ((y + z) & 1);
    return cube;
}
