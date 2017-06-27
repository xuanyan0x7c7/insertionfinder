#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "formula.h"

typedef struct Cube Cube;
struct Cube {
    int corner[8];
    int edge[12];
};

Cube* CubeConstruct(Cube* cube, const Formula* formula);

void CubeSave(const Cube* cube, FILE* stream);
Cube* CubeLoad(Cube* cube, FILE* stream);

void CubeTwist(Cube* cube, int move);
void CubeTwistCorner(Cube* cube, int move);
void CubeTwistEdge(Cube* cube, int move);
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
void CubeTwistCube(Cube* cube, const Cube* state);

unsigned CubeMask(const Cube* cube);
