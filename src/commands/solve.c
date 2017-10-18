#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-glib/json-glib.h>
#include <config.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../data-structure/hash-map.h"
#include "../finder/finder.h"
#include "../formula/formula.h"
#include "../utils/memory.h"
#include "../utils/io.h"
#include "utils.h"
#include "commands.h"


typedef FinderWorker Worker;

typedef struct SolvingFunctionArgs SolvingFunctionArgs;
struct SolvingFunctionArgs {
    Finder* finder;
    const Formula* skeleton;
    size_t max_threads;
};

typedef void SolvingFunction(
    SolvingFunctionArgs* args,
    FinderSolveResult* return_value
);

typedef void OutputFunction(
    const Formula*, const Formula*,
    bool, int, int,
    Finder*,
    SolvingFunction*, SolvingFunctionArgs*
);


static FILE* open_algorithm_file(const char* path);

static void solving(SolvingFunctionArgs* args, FinderSolveResult* return_value);

static void standard_output(
    const Formula* scramble, const Formula* skeleton,
    bool parity,
    int corner_cycles, int edge_cycles,
    Finder* finder,
    SolvingFunction* solve, SolvingFunctionArgs* args
);

static void json_output(
    const Formula* scramble, const Formula* skeleton,
    bool parity,
    int corner_cycles, int edge_cycles,
    Finder* finder,
    SolvingFunction* solve, SolvingFunctionArgs* args
);

static void solution_to_json(const Worker* solution, JsonArray* solution_array);


bool solve(const CliParser* parsed_args) {
    HashMap map;
    hashmap_construct(
        &map,
        cube_equal_generic,
        cube_hash_generic,
        NULL,
        algorithm_free_generic
    );

    for (size_t i = 0; i < parsed_args->algfile_count; ++i) {
        const char* path = parsed_args->algfile_list[i];
        FILE* algorithm_file = open_algorithm_file(path);
        if (!algorithm_file) {
            fprintf(stderr, "Cannot open algorithm file: %s\n", path);
            continue;
        }
        size_t size;
        if (!safe_read(&size, sizeof(size_t), 1, algorithm_file)) {
            fprintf(stderr, "Invalid algorithm file: %s\n", path);
            continue;
        }
        for (size_t j = 0; j < size; ++j) {
            Algorithm* algorithm = MALLOC(Algorithm);
            if (!algorithm_load(algorithm, algorithm_file)) {
                fprintf(stderr, "Invalid algorithm file: %s\n", path);
                free(algorithm);
                break;
            }
            HashMapNode* node;
            if (!hashmap_insert(&map, &algorithm->state, algorithm, &node)) {
                Algorithm* dest = (Algorithm*)node->value;
                for (size_t k = 0; k < algorithm->size; ++k) {
                    algorithm_add_formula(dest, &algorithm->formula_list[k]);
                }
                algorithm_destroy(algorithm);
                free(algorithm);
            }
        }
        fclose(algorithm_file);
    }

    Algorithm** algorithm_list = MALLOC(Algorithm*, map.size);
    size_t index = 0;
    for (
        HashMapNode* node = hashmap_iter_start(&map);
        node;
        node = hashmap_iter_next(&map, node)
    ) {
        algorithm_list[index] = (Algorithm*)node->value;
        algorithm_sort_formula(algorithm_list[index]);
        ++index;
    }
    qsort(
        algorithm_list,
        map.size,
        sizeof(Algorithm*),
        algorithm_compare_generic
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
            (scramble_string = get_line(input))
            && (skeleton_string = get_line(input))
        )) {
            fputs("Error input\n", stderr);
            success = false;
            break;
        }

        Formula scramble;
        Formula skeleton;
        if (!formula_construct(&scramble, scramble_string)) {
            fprintf(stderr, "Invalid scramble sequence: %s\n", scramble_string);
            success = false;
            break;
        }
        if (!formula_construct(&skeleton, skeleton_string)) {
            fprintf(
                stderr,
                "Invalid skeleton sequence: %s\n",
                skeleton_string
            );
            success = false;
            break;
        }

        Cube cube = identity_cube;
        cube_twist_formula(&cube, &scramble, true, true, false);
        cube_twist_formula(&cube, &skeleton, true, true, false);
        bool parity = cube_has_parity(&cube);
        int corner_cycles = cube_corner_cycles(&cube);
        int edge_cycles = cube_edge_cycles(&cube);

        Finder finder;
        finder_construct(&finder, map.size, algorithm_list, &scramble);
        SolvingFunctionArgs args = {
            .finder = &finder,
            .skeleton = &skeleton,
            .max_threads = parsed_args->max_threads
        };
        OutputFunction* print =
            parsed_args->json ? json_output : standard_output;
        print(
            &scramble, &skeleton,
            parity, corner_cycles, edge_cycles,
            &finder,
            solving, &args
        );

        formula_destroy(&scramble);
        formula_destroy(&skeleton);
        finder_destroy(&finder);

        if (input != stdin) {
            fclose(input);
        }
    } while (false);

    hashmap_destroy(&map);
    free(algorithm_list);
    free(scramble_string);
    free(skeleton_string);
    return success;
}


FILE* open_algorithm_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file) {
        return file;
    }
    char shared_path[
        strlen(ALGORITHMSDIR "/") + strlen(path) + strlen(".algs") + 1
    ];
    strcpy(shared_path, ALGORITHMSDIR "/");
    strcat(shared_path, path);
    strcat(shared_path, ".algs");
    return fopen(shared_path, "rb");
}


void solving(SolvingFunctionArgs* args, FinderSolveResult* return_value) {
    *return_value = finder_solve(
        args->finder,
        args->skeleton,
        args->max_threads
    );
}


void standard_output(
    const Formula* scramble, const Formula* skeleton,
    bool parity,
    int corner_cycles, int edge_cycles,
    Finder* finder,
    SolvingFunction* solve, SolvingFunctionArgs* args
) {
    printf("Scramble: ");
    formula_print(scramble, stdout);
    putchar('\n');
    printf("Skeleton: ");
    formula_print(skeleton, stdout);
    putchar('\n');

    if (corner_cycles == 0 && edge_cycles == 0) {
        if (parity) {
            puts("The cube has parity with no additional cycles.");
        } else {
            puts("The cube is solved.");
        }
    } else {
        if (parity) {
            printf("The cube has parity, with additional ");
        } else {
            printf("The cube has ");
        }
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

    FinderSolveResult result;
    solve(args, &result);
    switch (result.status) {
        case SOLVE_SUCCESS:
            if (finder->solution_count == 0) {
                puts("No solution found.");
            }
            for (size_t i = 0; i < finder->solution_count; ++i) {
                printf("\nSolution #%zu:\n", i + 1);
                const Worker* solution = &finder->solution_list[i];
                for (size_t j = 0; j < solution->depth; ++j) {
                    const Insertion* insertion = &solution->solving_step[j];
                    const Formula* skeleton = &insertion->skeleton;
                    size_t insert_place = insertion->insert_place;
                    if (insert_place > 0) {
                        formula_print_range(
                            skeleton,
                            0, insert_place,
                            stdout
                        );
                        putchar(' ');
                    }
                    printf("[@%zu]", j + 1);
                    if (insert_place < skeleton->length) {
                        putchar(' ');
                        formula_print_range(
                            skeleton,
                            insert_place, skeleton->length,
                            stdout
                        );
                    }
                    printf("\nAt @%zu insert: ", j + 1);
                    formula_print(insertion->insertion, stdout);
                    putchar('\n');
                }
                printf(
                    "Total moves: %zu,  %zu move%s cancelled.\n",
                    finder->fewest_moves,
                    solution->cancellation,
                    solution->cancellation == 1 ? "" : "s"
                );
                printf("Full solution: ");
                formula_print(
                    &solution->solving_step[solution->depth].skeleton,
                    stdout
                );
                printf("\n");
            }
            break;
        case SOLVE_FAILURE_PARITY_ALGORITHMS_NEEDED:
            puts("Parity algorithms needed.");
            break;
        case SOLVE_FAILURE_CORNER_CYCLE_ALGORITHMS_NEEDED:
            puts("Corner 3-cycle algorithms needed.");
            break;
        case SOLVE_FAILURE_EDGE_CYCLE_ALGORITHMS_NEEDED:
            puts("Edge 3-cycle algorithms needed.");
            break;
        default:
            break;
    }
    if (result.duration < 1000) {
        printf("Time usage: %" PRId64 " nanoseconds.\n", result.duration);
    } else if (result.duration < 1000000) {
        printf("Time usage: %.3f microseconds.\n", result.duration / 1e3);
    } else if (result.duration < 1000000000) {
        printf("Time usage: %.3f milliseconds.\n", result.duration / 1e6);
    } else if (result.duration < 60 * (int64_t)1000000000) {
        printf("Time usage: %.3f seconds.\n", result.duration / 1e9);
    } else if (result.duration < 60 * 60 * (int64_t)1000000000) {
        printf(
            "Time usage: %" PRId64 ":%02" PRId64 ".%3" PRId64 ".\n",
            result.duration / (60 * (int64_t)1000000000),
            (result.duration / 1000000000) % 60,
            (result.duration % 1000000000 + 500000) / 1000000
        );
    } else {
        printf(
            "Time usage: %" PRId64 ":%02" PRId64
            ":%02" PRId64 ".%3" PRId64 ".\n",
            result.duration / (60 * 60 * (int64_t)1000000000),
            (result.duration / (60 * (int64_t)1000000000)) % 60,
            (result.duration / 1000000000) % 60,
            (result.duration % 1000000000 + 500000) / 1000000
        );
    }
}


void json_output(
    const Formula* scramble, const Formula* skeleton,
    bool parity,
    int corner_cycles, int edge_cycles,
    Finder* finder,
    SolvingFunction* solve, SolvingFunctionArgs* args
) {
    FinderSolveResult result;
    solve(args, &result);

    JsonObject* object = json_object_new();

    char* scramble_string = formula_to_string(scramble);
    json_object_set_string_member(object, "scramble", scramble_string);
    free(scramble_string);
    json_object_set_int_member(object, "scramble_moves", scramble->length);

    char* skeleton_string = formula_to_string(skeleton);
    json_object_set_string_member(object, "skeleton", skeleton_string);
    free(skeleton_string);
    json_object_set_int_member(object, "skeleton_moves", skeleton->length);

    json_object_set_boolean_member(object, "parity", parity);
    json_object_set_int_member(object, "corner_cycle_num", corner_cycles);
    json_object_set_int_member(object, "edge_cycle_num", edge_cycles);
    if (finder->solution_count) {
        json_object_set_int_member(
            object,
            "minimum_moves",
            finder->fewest_moves
        );
    } else if (!parity && corner_cycles == 0 && edge_cycles == 0) {
        json_object_set_int_member(object, "minimum_moves", 0);
    } else {
        json_object_set_null_member(object, "minimum_moves");
    }

    JsonArray* solution_array = json_array_new();
    for (size_t i = 0; i < finder->solution_count; ++i) {
        solution_to_json(&finder->solution_list[i], solution_array);
    }
    json_object_set_array_member(object, "solution", solution_array);
    json_object_set_int_member(object, "duration", result.duration);

    JsonNode* json = json_node_alloc();
    json_node_init_object(json, object);
    json_object_unref(object);

    print_json(json, stdout);
    json_node_unref(json);
}


void solution_to_json(const Worker* solution, JsonArray* solution_array) {
    JsonObject* object = json_object_new();

    char* final_solution = formula_to_string(
        &solution->solving_step[solution->depth].skeleton
    );
    json_object_set_string_member(object, "final_solution", final_solution);
    free(final_solution);
    json_object_set_int_member(object, "cancellation", solution->cancellation);

    JsonArray* solving_step_array = json_array_new();

    for (size_t j = 0; j < solution->depth; ++j) {
        const Insertion* insertion = &solution->solving_step[j];
        const Formula* skeleton = &insertion->skeleton;
        size_t insert_place = insertion->insert_place;

        JsonObject* solving_step_object = json_object_new();

        char* skeleton_string;
        size_t temp_size;
        FILE* skeleton_stream = open_memstream(&skeleton_string, &temp_size);
        if (insert_place > 0) {
            formula_print_range(skeleton, 0, insert_place, skeleton_stream);
            fputc(' ', skeleton_stream);
        }
        fputc('@', skeleton_stream);
        if (insert_place < skeleton->length) {
            fputc(' ', skeleton_stream);
            formula_print_range(
                skeleton,
                insert_place, skeleton->length,
                skeleton_stream
            );
        }
        fclose(skeleton_stream);
        json_object_set_string_member(
            solving_step_object,
            "skeleton",
            skeleton_string
        );
        free(skeleton_string);

        char* insertion_string = formula_to_string(insertion->insertion);
        json_object_set_string_member(
            solving_step_object,
            "insertion",
            insertion_string
        );
        free(insertion_string);

        json_array_add_object_element(solving_step_array, solving_step_object);
    }

    json_object_set_array_member(object, "insertions", solving_step_array);
    json_array_add_object_element(solution_array, object);
}
