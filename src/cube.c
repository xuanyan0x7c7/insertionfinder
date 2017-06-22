#include <stdlib.h>
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


void CubeTwist(Cube* cube, int move) {
    CubeTwistCorner(cube, move);
    CubeTwistEdge(cube, move);
}

void CubeTwistCorner(Cube* cube, int move) {
    for (int i = 0; i < 8; ++i) {
        int* item = &(cube->corner[i]);
        int transform = corner_twist_table[move][*item / 3];
        *item = transform - transform % 3 + (*item + transform) % 3;
    }
}

void CubeTwistEdge(Cube* cube, int move) {
    for (int i = 0; i < 12; ++i) {
        int *item = &(cube->edge[i]);
        *item = edge_twist_table[move][*item >> 1] ^ (*item & 1);
    }
}

void CubeFormulaTwist(
    Cube* cube,
    const Formula* formula,
    bool twist_corners, bool twist_edges,
    bool reversed
) {
    CubeRangeTwistFormula(
        cube,
        formula,
        0, FormulaLength(formula),
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
            int move = inverse_move_table[FormulaGetMove(formula, i - 1)];
            if (twist_corners) {
                CubeTwistCorner(cube, move);
            }
            if (twist_edges) {
                CubeTwistEdge(cube, move);
            }
        }
    } else {
        for (size_t i = begin; i < end; ++i) {
            int move = FormulaGetMove(formula, i);
            if (twist_corners) {
                CubeTwistCorner(cube, move);
            }
            if (twist_edges) {
                CubeTwistEdge(cube, move);
            }
        }
    }
}
