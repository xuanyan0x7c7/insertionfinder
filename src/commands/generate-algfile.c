#include <stdio.h>
#include <stdlib.h>
#include "../data-structure/hash-map.h"
#include "../algorithm.h"
#include "../cube.h"
#include "utils.h"
#include "commands.h"


static bool CubeEqualGeneric(const void* p, const void* q);
static size_t CubeHashGeneric(const void* p);
static int AlgorithmCompareGeneric(const void* p, const void* q);
static void AlgorithmFreeGeneric(void* p);


void GenerateAlgfiles(const CliParser* parsed_args) {
    HashMap map;
    HashMapConstruct(
        &map,
        CubeEqualGeneric,
        CubeHashGeneric,
        free,
        AlgorithmFreeGeneric
    );

    for (size_t i = 0; i < parsed_args->file_count; ++i) {
        FILE* input = fopen(parsed_args->file_list[i], "r");
        if (!input) {
            fprintf(stderr, "Fail to open file: %s", parsed_args->file_list[i]);
            continue;
        }

        while (true) {
            char* string = GetLine(input);
            if (!string) {
                break;
            } else if (string[0] == '\0') {
                free(string);
                continue;
            }
            Formula formula;
            do {
                if (!FormulaConstruct(&formula, string)) {
                    fprintf(stderr, "Invalid formula: %s\n", string);
                    break;
                }
                Cube cube;
                CubeConstruct(&cube, &formula);
                if (CubeHasParity(&cube)) {
                    fprintf(stderr, "Formula has parity: %s\n", string);
                    break;
                } else if (CubeMask(&cube) == 0) {
                    fprintf(
                        stderr,
                        "Formula does not change the state: %s\n",
                        string
                    );
                    break;
                }
                FormulaNormalize(&formula);
                HashMapNode* node = HashMapFind(&map, &cube);
                if (node && AlgorithmContainsFormula(
                    (Algorithm*)node->value,
                    &formula
                )) {
                    break;
                }

                Formula isomorphism_list[96];
                size_t isomorphism_count = FormulaGenerateIsomorphisms(
                    &formula,
                    isomorphism_list
                );
                for (size_t i = 0; i < isomorphism_count; ++i) {
                    Formula* formula = &isomorphism_list[i];
                    Cube* cube = CubeConstruct(NULL, formula);
                    HashMapNode* node = HashMapFind(&map, cube);
                    if (node) {
                        Algorithm* algorithm = (Algorithm*)node->value;
                        AlgorithmAddFormula(algorithm, formula);
                        free(cube);
                    } else {
                        Algorithm* algorithm = AlgorithmConstruct(NULL, cube);
                        AlgorithmAddFormula(algorithm, formula);
                        HashMapInsert(&map, cube, algorithm);
                    }
                    FormulaDestroy(formula);
                }
            } while (false);
            free(string);
            FormulaDestroy(&formula);
        }

        fclose(input);
    }

    Algorithm** list = (Algorithm**)malloc(map.size * sizeof(Algorithm*));
    size_t index = 0;
    for (
        HashMapNode* node = HashMapIterStart(&map);
        node;
        node = HashMapIterNext(&map, node)
    ) {
        list[index++] = (Algorithm*)node->value;
    }
    qsort(list, map.size, sizeof(Algorithm*), AlgorithmCompareGeneric);

    fwrite(&map.size, sizeof(size_t), 1, stdout);
    for (size_t i = 0; i < map.size; ++i) {
        AlgorithmSave(list[i], stdout);
    }
    free(list);
}


bool CubeEqualGeneric(const void* p, const void* q) {
    const int* corner1 = ((const Cube*)p)->corner;
    const int* corner2 = ((const Cube*)q)->corner;
    const int* edge1 = ((const Cube*)p)->edge;
    const int* edge2 = ((const Cube*)q)->edge;
    for (int i = 0; i < 8; ++i) {
        if (corner1[i] != corner2[i]) {
            return false;
        }
    }
    for (int i = 0; i < 12; ++i) {
        if (edge1[i] != edge2[i]) {
            return false;
        }
    }
    return true;
}

size_t CubeHashGeneric(const void* p) {
    const int* corner = ((const Cube*)p)->corner;
    const int* edge = ((const Cube*)p)->edge;
    size_t hash = 0;
    for (int i = 0; i < 8; ++i) {
        hash = hash * 31 + corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        hash = hash * 31 + edge[i];
    }
    return hash;
}

int AlgorithmCompareGeneric(const void* p, const void* q) {
    return AlgorithmCompare(*(const Algorithm**)p, *(const Algorithm**)q);
}

void AlgorithmFreeGeneric(void* p) {
    Algorithm* algorithm = (Algorithm*)p;
    AlgorithmDestroy(algorithm);
    free(algorithm);
}
