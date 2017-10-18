#include <stdbool.h>
#include <string.h>
#include "../formula/formula.h"
#include "cube.h"
#include "common.h"


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


void cube_twist_move(
    Cube* cube,
    int move,
    bool twist_corners, bool twist_edges
) {
    typedef int Array[24];
    if (twist_corners) {
        Array* table = computed_corner_twist_table[move];
        for (int i = 0; i < 8; ++i) {
            cube->corner[i] = table[i][cube->corner[i]];
        }
    }
    if (twist_edges) {
        Array* table = computed_edge_twist_table[move];
        for (int i = 0; i < 12; ++i) {
            cube->edge[i] = table[i][cube->edge[i]];
        }
    }
}

void cube_twist_formula(
    Cube* cube,
    const Formula* formula,
    bool twist_corners, bool twist_edges,
    bool reversed
) {
    cube_range_twist_formula(
        cube,
        formula,
        0, formula->length,
        twist_corners, twist_edges,
        reversed
    );
}

void cube_range_twist_formula(
    Cube* cube,
    const Formula* formula,
    size_t begin, size_t end,
    bool twist_corners, bool twist_edges,
    bool reversed
) {
    if (reversed) {
        for (size_t i = end; i > begin; --i) {
            cube_twist_move(
                cube,
                inverse_move_table[formula->move[i - 1]],
                twist_corners, twist_edges
            );
        }
    } else {
        for (size_t i = begin; i < end; ++i) {
            cube_twist_move(cube, formula->move[i], twist_corners, twist_edges);
        }
    }
}

void cube_twist_cube(
    Cube* cube,
    const Cube* state,
    bool twist_corners, bool twist_edges
) {
    if (twist_corners) {
        int* begin = cube->corner;
        int* end = begin + 8;
        for (int* item = begin; item < end; ++item) {
            int transform = state->corner[*item / 3];
            *item = transform - transform % 3 + (*item + transform) % 3;
        }
    }
    if (twist_edges) {
        int* begin = cube->edge;
        int* end = begin + 12;
        for (int* item = begin; item < end; ++item) {
            *item = state->edge[*item >> 1] ^ (*item & 1);
        }
    }
}


void cube_twist_move_before(
    Cube* cube,
    int move,
    bool twist_corners, bool twist_edges
) {
    cube_twist_cube_before(
        cube,
        &one_move_cube[move],
        twist_corners, twist_edges
    );
}

void cube_twist_cube_before(
    Cube* cube,
    const Cube* state,
    bool twist_corners, bool twist_edges
) {
    if (twist_corners) {
        int corners[8];
        for (int i = 0; i < 8; ++i) {
            int item = state->corner[i];
            int transform = cube->corner[item / 3];
            corners[i] = transform - transform % 3 + (item + transform) % 3;
        }
        memcpy(cube->corner, corners, 8 * sizeof(int));
    }
    if (twist_edges) {
        int edges[12];
        for (int i = 0; i < 12; ++i) {
            int item = state->edge[i];
            edges[i] = cube->edge[item >> 1] ^ (item & 1);
        }
        memcpy(cube->edge, edges, 12 * sizeof(int));
    }
}


bool cube_twist_positive(
    Cube* cube,
    const Cube* c1, const Cube* c2,
    bool corner_changed, bool edge_changed
) {
    if (corner_changed) {
        for (int i = 0; i < 8; ++i) {
            int item = c1->corner[i];
            int transform = c2->corner[item / 3];
            int result = transform - transform % 3 + (item + transform) % 3;
            if (result / 3 == i && result % 3 && item != result) {
                return false;
            }
            cube->corner[i] = result;
        }
    }
    if (edge_changed) {
        for (int i = 0; i < 12; ++i) {
            int item = c1->edge[i];
            int result = c2->edge[item >> 1] ^ (item & 1);
            if (result == (i << 1 | 1) && item != result) {
                return false;
            }
            cube->edge[i] = result;
        }
    }
    return true;
}


void cube_generate_move_cube(Cube* cube_list) {
    for (int i = 0; i < 24; ++i) {
        memcpy(cube_list[i].corner, corner_twist_table[i], 8 * sizeof(int));
        memcpy(cube_list[i].edge, edge_twist_table[i], 12 * sizeof(int));
    }
}


void cube_generate_computed_corner_twist(int table[][8][24]) {
    for (int move = 0; move < 24; ++move) {
        if ((move & 3) == 0) {
            continue;
        }
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 24; ++j) {
                int transform = corner_twist_table[move][j / 3];
                table[move][i][j] = transform / 3 * 3 + (j + transform) % 3;
            }
        }
    }
}

void cube_generate_computed_edge_twist(int table[][12][24]) {
    for (int move = 0; move < 24; ++move) {
        if ((move & 3) == 0) {
            continue;
        }
        for (int i = 0; i < 12; ++i) {
            for (int j = 0; j < 24; ++j) {
                table[move][i][j] = edge_twist_table[move][j >> 1] ^ (j & 1);
            }
        }
    }
}
