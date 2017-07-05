#include <stdio.h>
#include <stdlib.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../data-structure/hash-map.h"
#include "utils.h"
#include "commands.h"


bool GenerateAlgfiles(const CliParser* parsed_args) {
    HashMap map;
    HashMapConstruct(
        &map,
        CubeEqualGeneric,
        CubeHashGeneric,
        free,
        AlgorithmFreeGeneric
    );

    const char* filepath = NULL;
    if (parsed_args->algfile_count) {
        filepath = parsed_args->algfile_list[0];
    }
    FILE* output = filepath ? fopen(filepath, "wb") : stdout;
    if (!output) {
        fprintf(stderr, "Fail to open output file: %s\n", filepath);
        return false;
    }

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
                Cube cube = identity_cube;
                CubeTwistFormula(&cube, &formula, true, true, false);
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
                    Cube* cube = (Cube*)malloc(sizeof(Cube));
                    CubeConstruct(cube);
                    CubeTwistFormula(cube, formula, true, true, false);
                    HashMapNode* node = HashMapFind(&map, cube);
                    if (node) {
                        Algorithm* algorithm = (Algorithm*)node->value;
                        AlgorithmAddFormula(algorithm, formula);
                        free(cube);
                    } else {
                        Algorithm* algorithm = (Algorithm*)malloc(
                            sizeof(Algorithm)
                        );
                        AlgorithmConstruct(algorithm, cube);
                        AlgorithmAddFormula(algorithm, formula);
                        HashMapInsert(&map, cube, algorithm, NULL);
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

    fwrite(&map.size, sizeof(size_t), 1, output);
    for (size_t i = 0; i < map.size; ++i) {
        AlgorithmSave(list[i], output);
    }
    free(list);
    if (output != stdout) {
        fclose(output);
    }

    return true;
}
