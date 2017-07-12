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

Cube identity_cube;

void CubeInit();

void CubeConstruct(Cube* cube);

void CubeSave(const Cube* cube, FILE* stream);
bool CubeLoad(Cube* cube, FILE* stream);

void CubeTwistMove(Cube* cube, int move, bool twist_corners, bool twist_edges);
void CubeTwistFormula(
    Cube* cube,
    const Formula* formula,
    bool twist_corners, bool twist_edges,
    bool reversed
);
void CubeRangeTwistFormula(
    Cube* cube,
    const Formula* formula,
    size_t begin, size_t end,
    bool twist_corners, bool twist_edges,
    bool reversed
);
void CubeTwistCube(
    Cube* cube,
    const Cube* state,
    bool twist_corners, bool twist_edges
);

void CubeTwistMoveBefore(
    Cube* cube,
    int move,
    bool twist_corners, bool twist_edges
);
void CubeTwistCubeBefore(
    Cube* cube,
    const Cube* state,
    bool twist_corners, bool twist_edges
);

bool CubeTwistPositive(
    Cube* cube,
    const Cube* c1, const Cube* c2,
    bool corner_changed, bool edge_changed
);

void CubeInverseState(const Cube* state, Cube* result);

uint32_t CubeMask(const Cube* cube);

bool CubeHasParity(const Cube* cube);
int CubeCornerCycles(const Cube* cube);
int CubeEdgeCycles(const Cube* cube);

int CubeCorner3CycleIndex(const Cube* cube);
int CubeEdge3CycleIndex(const Cube* cube);
int CubeCornerNext3CycleIndex(int index, int move);
int CubeEdgeNext3CycleIndex(int index, int move);
