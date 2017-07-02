#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../formula/formula.h"
#include "cube.h"
#include "common.h"


extern Cube one_move_cube[24];
extern int corner_cycle_transform_table[6 * 24 * 24][24];
extern int edge_cycle_transform_table[10 * 24 * 24][24];

static void GenerateOneMoveCube(Cube* cube_list);


void CubeInitialize() {
    GenerateOneMoveCube(one_move_cube);
    GenerateCornerCycleTable(corner_cycle_transform_table);
    GenerateEdgeCycleTable(edge_cycle_transform_table);
}


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
    if (formula) {
        CubeTwistFormula(cube, formula, true, true, false);
    }
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


void GenerateOneMoveCube(Cube* cube_list) {
    for (int i = 0; i < 24; ++i) {
        Cube* cube = &cube_list[i];
        CubeConstruct(cube, NULL);
        CubeTwist(cube, i, true, true);
    }
}
