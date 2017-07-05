#pragma once
#include "cube.h"

Cube one_move_cube[24];
int computed_corner_twist_table[24][8][24];
int computed_edge_twist_table[24][12][24];
int corner_cycle_transform_table[6 * 24 * 24][24];
int edge_cycle_transform_table[10 * 24 * 24][24];

void GenerateOneMoveCube(Cube* cube_list);
void GenerateComputedCornerTwistTable(int table[][8][24]);
void GenerateComputedEdgeTwistTable(int table[][12][24]);
void GenerateCornerCycleTable(int table[][24]);
void GenerateEdgeCycleTable(int table[][24]);
