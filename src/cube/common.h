#pragma once
#include "cube.h"

Cube one_move_cube[24];
int computed_corner_twist_table[24][8][24];
int computed_edge_twist_table[24][12][24];
int parity_transform_table[7 * 24 * 11 * 24][24];
int corner_cycle_transform_table[6 * 24 * 24][24];
int edge_cycle_transform_table[10 * 24 * 24][24];

void cube_generate_move_cube(Cube* cube_list);
void cube_generate_computed_corner_twist(int table[][8][24]);
void cube_generate_computed_edge_twist(int table[][12][24]);
void cube_generate_parity(int table[][24]);
void cube_generate_corner_cycle(int table[][24]);
void cube_generate_edge_cycle(int table[][24]);
