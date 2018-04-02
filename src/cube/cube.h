#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "../formula/formula.h"

typedef struct Cube Cube;
struct Cube {
    int corner[8];
    int edge[12];
};

extern Cube identity_cube;

void cube_init();

void cube_construct(Cube* cube);

void cube_save(const Cube* cube, FILE* stream);
bool cube_load(Cube* cube, FILE* stream);

void cube_twist_move(
    Cube* cube,
    int move,
    bool twist_corners, bool twist_edges
);
void cube_twist_formula(
    Cube* cube,
    const Formula* formula,
    bool twist_corners, bool twist_edges,
    bool reversed
);
void cube_range_twist_formula(
    Cube* cube,
    const Formula* formula,
    size_t begin, size_t end,
    bool twist_corners, bool twist_edges,
    bool reversed
);
void cube_twist_cube(
    Cube* cube,
    const Cube* state,
    bool twist_corners, bool twist_edges
);

void cube_twist_move_before(
    Cube* cube,
    int move,
    bool twist_corners, bool twist_edges
);
void cube_twist_cube_before(
    Cube* cube,
    const Cube* state,
    bool twist_corners, bool twist_edges
);

bool cube_twist_positive(
    Cube* cube,
    const Cube* c1, const Cube* c2,
    bool corner_changed, bool edge_changed
);

void cube_inverse_state(const Cube* state, Cube* result);

uint32_t cube_mask(const Cube* cube);

bool cube_has_parity(const Cube* cube);
int cube_corner_cycles(const Cube* cube);
int cube_edge_cycles(const Cube* cube);

int cube_parity_index(const Cube* cube);
int cube_corner_cycle_index(const Cube* cube);
int cube_edge_cycle_index(const Cube* cube);
int cube_parity_next_index(int index, int move);
int cube_corner_next_cycle_index(int index, int move);
int cube_edge_next_cycle_index(int index, int move);
