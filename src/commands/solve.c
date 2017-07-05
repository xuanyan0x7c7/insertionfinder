#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../data-structure/hash-map.h"
#include "../finder/finder.h"
#include "../formula/formula.h"
#include "utils.h"
#include "commands.h"


bool Solve(const CliParser* parsed_args) {
    HashMap map;
    HashMapConstruct(
        &map,
        CubeEqualGeneric,
        CubeHashGeneric,
        NULL,
        AlgorithmFreeGeneric
    );

    for (size_t i = 0; i < parsed_args->algfile_count; ++i) {
        const char* path = parsed_args->algfile_list[i];
        FILE* algorithm_file = fopen(path, "r");
        if (!algorithm_file) {
            const char* prefix = PREFIX "/share/"
                PACKAGE_NAME "/" PACKAGE_VERSION "/algorithms/";
            char shared_path[strlen(prefix) + strlen(path) + 6];
            strcpy(shared_path, prefix);
            strcat(shared_path, path);
            strcat(shared_path, ".algs");
            algorithm_file = fopen(shared_path, "r");
            if (!algorithm_file) {
                fprintf(stderr, "Cannot open algorithm file: %s\n", path);
                continue;
            }
        }
        size_t size;
        fread(&size, sizeof(size_t), 1, algorithm_file);
        for (size_t j = 0; j < size; ++j) {
            Algorithm* algorithm = (Algorithm*)malloc(sizeof(Algorithm));
            AlgorithmLoad(algorithm, algorithm_file);
            HashMapNode* node;
            if (!HashMapInsert(&map, &algorithm->state, algorithm, &node)) {
                Algorithm* dest = (Algorithm*)node->value;
                for (size_t k = 0; k < algorithm->size; ++k) {
                    const Formula* formula = &algorithm->formula_list[k];
                    if (!AlgorithmContainsFormula(dest, formula)) {
                        AlgorithmAddFormula(dest, formula);
                    }
                }
                AlgorithmDestroy(algorithm);
                free(algorithm);
            }
        }
        fclose(algorithm_file);
    }

    Algorithm** algorithm_list = (Algorithm**)malloc(
        map.size * sizeof(Algorithm*)
    );
    size_t index = 0;
    for (
        HashMapNode* node = HashMapIterStart(&map);
        node;
        node = HashMapIterNext(&map, node)
    ) {
        algorithm_list[index++] = (Algorithm*)node->value;
    }
    qsort(
        algorithm_list,
        map.size,
        sizeof(Algorithm*),
        AlgorithmCompareGeneric
    );

    const char* filepath = NULL;
    if (parsed_args->casefile_count) {
        filepath = parsed_args->casefile_list[0];
    }
    char* scramble_string = NULL;
    char* skeleton_string = NULL;
    FILE* input = filepath ? fopen(filepath, "r") : stdin;
    if (!input) {
        fprintf(stderr, "Fail to open file: %s\n", filepath);
        return false;
    }

    bool success = true;
    do {
        if (!(
            (scramble_string = GetLine(input))
            && (skeleton_string = GetLine(input))
        )) {
            fputs("Error input\n", stderr);
            success = false;
            break;
        }

        Formula scramble;
        Formula skeleton;
        if (!FormulaConstruct(&scramble, scramble_string)) {
            fprintf(stderr, "Invalid scramble sequence: %s\n", scramble_string);
            success = false;
            break;
        }
        if (!FormulaConstruct(&skeleton, skeleton_string)) {
            fprintf(
                stderr,
                "Invalid skeleton sequence: %s\n",
                skeleton_string
            );
            success = false;
            break;
        }

        if (parsed_args->json) {
            printf("{\"scramble\":\"");
            FormulaPrint(&scramble, stdout);
            printf("\",\"scramble_moves\":%lu,", scramble.length);
            printf("\"skeleton\":\"");
            FormulaPrint(&skeleton, stdout);
            printf("\",\"skeleton_moves\":%lu,", skeleton.length);
        } else {
            printf("Scramble: ");
            FormulaPrint(&scramble, stdout);
            putchar('\n');
            printf("Skeleton: ");
            FormulaPrint(&skeleton, stdout);
            putchar('\n');
        }

        Finder finder;
        FinderConstruct(&finder, map.size, algorithm_list, &scramble);
        bool parity;
        int corner_cycles;
        int edge_cycles;
        switch (FinderSolve(
            &finder,
            &skeleton,
            &parity,
            &corner_cycles, &edge_cycles
        )) {
            case SOLVE_SUCCESS:
                if (parsed_args->json) {
                    printf(
                        "\"corner_cycle_num\":%d,"
                        "\"edge_cycle_num\":%d,",
                        corner_cycles, edge_cycles
                    );
                } else {
                    printf("The cube has ");
                    if (corner_cycles == 1) {
                        printf("1 corner-3-cycle");
                    } else if (corner_cycles) {
                        printf("%d corner-3-cycles", corner_cycles);
                    }
                    if (corner_cycles && edge_cycles) {
                        printf(" and ");
                    }
                    if (edge_cycles == 1) {
                        printf("1 edge-3-cycle");
                    } else if (edge_cycles) {
                        printf("%d edge-3-cycles", edge_cycles);
                    }
                    printf(".\n");
                }
                if (finder.solution_count == 0) {
                    if (parsed_args->json) {
                        printf("\"solutions\":[]}");
                    } else {
                        puts("No solution found.");
                    }
                    break;
                }
                if (parsed_args->json) {
                    printf("\"solutions\":[");
                }
                for (size_t i = 0; i < finder.solution_count; ++i) {
                    if (parsed_args->json) {
                        if (i > 0) {
                            putchar(',');
                        }
                        printf("{\"insertions\":[");
                    } else {
                        printf("\nSolution #%lu:\n", i + 1);
                    }
                    const FinderWorker* solution = &finder.solution_list[i];
                    for (size_t j = 0; j < solution->depth; ++j) {
                        const Insertion* insertion = &solution->solving_step[j];
                        const Formula* skeleton = &insertion->skeleton;
                        size_t insert_place = insertion->insert_place;
                        if (parsed_args->json) {
                            if (j > 0) {
                                putchar(',');
                            }
                            printf("{\"skeleton\":\"");
                        }
                        if (insert_place > 0) {
                            FormulaPrintRange(
                                skeleton,
                                0, insert_place,
                                stdout
                            );
                            putchar(' ');
                        }
                        if (parsed_args->json) {
                            putchar('@');
                        } else {
                            printf("[@%lu]", j + 1);
                        }
                        if (insert_place < skeleton->length) {
                            putchar(' ');
                            FormulaPrintRange(
                                skeleton,
                                insert_place, skeleton->length,
                                stdout
                            );
                        }
                        if (parsed_args->json) {
                            printf("\",\"insertion\":\"");
                        } else {
                            printf("\nInsert at @%lu: ", j + 1);
                        }
                        FormulaPrint(insertion->insertion, stdout);
                        if (parsed_args->json) {
                            printf("\"}");
                        } else {
                            putchar('\n');
                        }
                    }
                    if (parsed_args->json) {
                        printf(
                            "],"
                            "\"cancellation\":%lu,"
                            "\"final_solution\":\"",
                            solution->cancellation
                        );
                    } else {
                        printf(
                            "Total moves: %lu,  %lu move%s cancelled.\n",
                            finder.fewest_moves,
                            solution->cancellation,
                            solution->cancellation == 1 ? "" : "s"
                        );
                        printf("Full solution: ");
                    }
                    FormulaPrint(
                        &solution->solving_step[solution->depth].skeleton,
                        stdout
                    );
                    if (parsed_args->json) {
                        printf("\"}");
                    } else {
                        printf("\n");
                    }
                }
                if (parsed_args->json) {
                    printf("]}");
                }
                break;
            case SOLVE_SUCCESS_SOLVED:
                if (parsed_args->json) {
                    printf(
                        "\"corner_cycle_num\":0,"
                        "\"edge_cycle_num\":0,"
                        "\"minimum_moves\":%lu,"
                        "\"solutions\":\"[]\"}",
                        skeleton.length
                    );
                } else {
                    puts("The cube is solved.");
                }
                break;
            case SOLVE_FAILURE_PARITY:
                if (parsed_args->json) {
                    printf("\"parity\":true}");
                } else {
                    puts("The cube has parity.");
                }
                break;
            case SOLVE_FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED:
                if (parsed_args->json) {
                    printf(
                        "\"corner_cycle_num\":%d,"
                        "\"edge_cycle_num\":%d}",
                        corner_cycles, edge_cycles
                    );
                } else {
                    printf("The cube has ");
                    if (corner_cycles == 1) {
                        printf("1 corner-3-cycle");
                    } else if (corner_cycles) {
                        printf("%d corner-3-cycles", corner_cycles);
                    }
                    if (corner_cycles && edge_cycles) {
                        printf(" and ");
                    }
                    if (edge_cycles == 1) {
                        printf("1 edge-3-cycle");
                    } else if (edge_cycles) {
                        printf("%d edge-3-cycles", edge_cycles);
                    }
                    puts(".");
                    puts("Corner 3-cycle algorithms needed.");
                }
                break;
            case SOLVE_FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED:
                if (parsed_args->json) {
                    printf(
                        "\"corner_cycle_num\":%d,"
                        "\"edge_cycle_num\":%d}",
                        corner_cycles, edge_cycles
                    );
                } else {
                    printf("The cube has ");
                    if (corner_cycles == 1) {
                        printf("1 corner-3-cycle");
                    } else if (corner_cycles) {
                        printf("%d corner-3-cycles", corner_cycles);
                    }
                    if (corner_cycles && edge_cycles) {
                        printf(" and ");
                    }
                    if (edge_cycles == 1) {
                        printf("1 edge-3-cycle");
                    } else if (edge_cycles) {
                        printf("%d edge-3-cycles", edge_cycles);
                    }
                    puts(".");
                    puts("Edge 3-cycle algorithms needed.");
                }
                break;
        }

        FinderDestroy(&finder);

        if (input != stdin) {
            fclose(input);
        }
    } while (false);

    HashMapDestroy(&map);
    free(algorithm_list);
    free(scramble_string);
    free(skeleton_string);
    return success;
}
