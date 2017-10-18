#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../formula/formula.h"
#include "../utils/io.h"
#include "cube.h"
#include "common.h"


void cube_init() {
    for (int i = 0; i < 8; ++i) {
        identity_cube.corner[i] = i * 3;
    }
    for (int i = 0; i < 12; ++i) {
        identity_cube.edge[i] = i << 1;
    }
    cube_generate_move_cube(one_move_cube);
    cube_generate_computed_corner_twist(computed_corner_twist_table);
    cube_generate_computed_edge_twist(computed_edge_twist_table);
    cube_generate_parity(parity_transform_table);
    cube_generate_corner_cycle(corner_cycle_transform_table);
    cube_generate_edge_cycle(edge_cycle_transform_table);
}


void cube_construct(Cube* cube) {
    memcpy(cube, &identity_cube, sizeof(Cube));
}


void cube_save(const Cube* cube, FILE* stream) {
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

bool cube_load(Cube* cube, FILE* stream) {
    int8_t corner[8], edge[12];
    if (!safe_read(corner, sizeof(int8_t), 8, stream)) {
        return false;
    }
    if (!safe_read(edge, sizeof(int8_t), 12, stream)) {
        return false;
    }
    for (int i = 0; i < 8; ++i) {
        cube->corner[i] = corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        cube->edge[i] = edge[i];
    }
    return true;
}


void cube_inverse_state(const Cube* state, Cube* result) {
    for (int i = 0; i < 8; ++i) {
        int item = state->corner[i];
        result->corner[item / 3] = i * 3 + (24 - item) % 3;
    }
    for (int i = 0; i < 12; ++i) {
        int item = state->edge[i];
        result->edge[item >> 1] = i << 1 | (item & 1);
    }
}


uint32_t cube_mask(const Cube* cube) {
    uint32_t mask = 0;
    for (int i = 0; i < 8; ++i) {
        mask = mask << 1 | (cube->corner[i] != i * 3);
    }
    for (int i = 0; i < 12; ++i) {
        mask = mask << 1 | (cube->edge[i] != i << 1);
    }
    return mask;
}
