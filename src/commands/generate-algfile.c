#include <stdio.h>
#include <stdlib.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../data-structure/hash-map.h"
#include "../utils/memory.h"
#include "utils.h"
#include "commands.h"


bool generate_alg_files(const CliParser* parsed_args) {
    HashMap map;
    hashmap_construct(
        &map,
        cube_equal_generic,
        cube_hash_generic,
        free,
        algorithm_free_generic
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
            char* string = get_line(input);
            if (!string) {
                break;
            } else if (string[0] == '\0') {
                free(string);
                continue;
            }
            Formula formula;
            do {
                if (!formula_construct(&formula, string)) {
                    fprintf(stderr, "Invalid formula: %s\n", string);
                    break;
                }
                Cube cube = identity_cube;
                cube_twist_formula(&cube, &formula, true, true, false);
                if (cube_mask(&cube) == 0) {
                    fprintf(
                        stderr,
                        "Formula does not change the state: %s\n",
                        string
                    );
                    break;
                }
                formula_normalize(&formula);
                HashMapNode* node = hashmap_find(&map, &cube);
                if (node && algorithm_contains_formula(
                    (Algorithm*)node->value,
                    &formula
                )) {
                    break;
                }

                Formula isomorphism_list[96];
                size_t isomorphism_count = formula_generate_isomorphisms(
                    &formula,
                    isomorphism_list
                );
                for (size_t i = 0; i < isomorphism_count; ++i) {
                    Formula* formula = &isomorphism_list[i];
                    Cube* cube = MALLOC(Cube);
                    cube_construct(cube);
                    cube_twist_formula(cube, formula, true, true, false);
                    HashMapNode* node = hashmap_find(&map, cube);
                    if (node) {
                        Algorithm* algorithm = (Algorithm*)node->value;
                        algorithm_add_formula(algorithm, formula);
                        free(cube);
                    } else {
                        Algorithm* algorithm = MALLOC(Algorithm);
                        algorithm_construct(algorithm, cube);
                        algorithm_add_formula(algorithm, formula);
                        hashmap_insert(&map, cube, algorithm, NULL);
                    }
                    formula_destroy(formula);
                }
            } while (false);
            free(string);
            formula_destroy(&formula);
        }

        fclose(input);
    }

    Algorithm** list = MALLOC(Algorithm*, map.size);
    size_t index = 0;
    for (
        HashMapNode* node = hashmap_iter_start(&map);
        node;
        node = hashmap_iter_next(&map, node)
    ) {
        list[index++] = (Algorithm*)node->value;
    }
    qsort(list, map.size, sizeof(Algorithm*), algorithm_compare_generic);

    fwrite(&map.size, sizeof(size_t), 1, output);
    for (size_t i = 0; i < map.size; ++i) {
        algorithm_sort_formula(list[i]);
        algorithm_save(list[i], output);
    }
    free(list);
    if (output != stdout) {
        fclose(output);
    }

    return true;
}
