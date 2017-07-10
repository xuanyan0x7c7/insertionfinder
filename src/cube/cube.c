#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../formula/formula.h"
#include "cube.h"
#include "common.h"


void CubeInit() {
    for (int i = 0; i < 8; ++i) {
        identity_cube.corner[i] = i * 3;
    }
    for (int i = 0; i < 12; ++i) {
        identity_cube.edge[i] = i << 1;
    }
    GenerateOneMoveCube(one_move_cube);
    GenerateComputedCornerTwistTable(computed_corner_twist_table);
    GenerateComputedEdgeTwistTable(computed_edge_twist_table);
    GenerateCornerCycleTable(corner_cycle_transform_table);
    GenerateEdgeCycleTable(edge_cycle_transform_table);
}


void CubeConstruct(Cube* cube) {
    memcpy(cube, &identity_cube, sizeof(Cube));
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

void CubeLoad(Cube* cube, FILE* stream) {
    int8_t corner[8], edge[12];
    fread(corner, sizeof(int8_t), 8, stream);
    fread(edge, sizeof(int8_t), 12, stream);
    for (int i = 0; i < 8; ++i) {
        cube->corner[i] = corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        cube->edge[i] = edge[i];
    }
}


void CubeInverseState(const Cube* state, Cube* result) {
    for (int i = 0; i < 8; ++i) {
        int item = state->corner[i];
        result->corner[item / 3] = i * 3 + (24 - item) % 3;
    }
    for (int i = 0; i < 12; ++i) {
        int item = state->edge[i];
        result->edge[item >> 1] = (i << 1) | (item & 1);
    }
}


uint32_t CubeMask(const Cube* cube) {
    uint32_t mask = 0;
    for (int i = 0; i < 8; ++i) {
        mask = (mask << 1) | (cube->corner[i] != i * 3);
    }
    for (int i = 0; i < 12; ++i) {
        mask = (mask << 1) | (cube->edge[i] != i << 1);
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
