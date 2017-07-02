#pragma once
#include "cube.h"

Cube one_move_cube[24];
int corner_cycle_transform_table[6 * 24 * 24][24];
int edge_cycle_transform_table[10 * 24 * 24][24];

void GenerateCornerCycleTable(int table[][24]);
void GenerateEdgeCycleTable(int table[][24]);
