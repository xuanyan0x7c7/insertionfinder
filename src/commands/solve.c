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
    FinderSolveStatus* return_value
);

typedef void OutputFunction(
    const Formula*, const Formula*,
    bool, int, int,
    Finder*,
    SolvingFunction*, SolvingFunctionArgs*
);


static FILE* OpenAlgorithmFile(const char* path);

static void Solving(SolvingFunctionArgs* args, FinderSolveStatus* return_value);

static void StandardOutput(
    const Formula* scramble, const Formula* skeleton,
    bool parity,
    int corner_cycles, int edge_cycles,
    Finder* finder,
    SolvingFunction* solve, SolvingFunctionArgs* args
);

static void JSONOutput(
    const Formula* scramble, const Formula* skeleton,
    bool parity,
    int corner_cycles, int edge_cycles,
    Finder* finder,
    SolvingFunction* solve, SolvingFunctionArgs* args
);

static void Solution2JSON(
    const Worker* solution,
    JsonArray* solution_array
);


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
        FILE* algorithm_file = OpenAlgorithmFile(path);
        if (!algorithm_file) {
            fprintf(stderr, "Cannot open algorithm file: %s\n", path);
            continue;
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
                    AlgorithmAddFormula(dest, &algorithm->formula_list[k]);
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
        algorithm_list[index] = (Algorithm*)node->value;
        AlgorithmSortFormula(algorithm_list[index]);
        ++index;
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

        Cube cube = identity_cube;
        CubeTwistFormula(&cube, &scramble, true, true, false);
        CubeTwistFormula(&cube, &skeleton, true, true, false);
        OutputFunction* print;
        bool parity = CubeHasParity(&cube);
        int corner_cycles;
        int edge_cycles;
        if (!parity) {
            corner_cycles = CubeCornerCycles(&cube);
            edge_cycles = CubeEdgeCycles(&cube);
        }

        Finder finder;
        FinderConstruct(&finder, map.size, algorithm_list, &scramble);
        print = parsed_args->json ? JSONOutput : StandardOutput;
        SolvingFunctionArgs args = {
            .finder = &finder,
            .skeleton = &skeleton,
            .max_threads = parsed_args->max_threads
        };
        print(
            &scramble, &skeleton,
            parity, corner_cycles, edge_cycles,
            &finder,
            Solving, &args
        );

        FormulaDestroy(&scramble);
        FormulaDestroy(&skeleton);
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


FILE* OpenAlgorithmFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file) {
        return file;
    }
    static const char* prefix = PREFIX "/share/"
        PACKAGE_NAME "/" PACKAGE_VERSION "/algorithms/";
    char shared_path[strlen(prefix) + strlen(path) + 6];
    strcpy(shared_path, prefix);
    strcat(shared_path, path);
    strcat(shared_path, ".algs");
    return fopen(shared_path, "rb");
}


void Solving(SolvingFunctionArgs* args, FinderSolveStatus* return_value) {
    *return_value = FinderSolve(
        args->finder,
        args->skeleton,
        args->max_threads
    );
}


void StandardOutput(
    const Formula* scramble, const Formula* skeleton,
    bool parity,
    int corner_cycles, int edge_cycles,
    Finder* finder,
    SolvingFunction* solve, SolvingFunctionArgs* args
) {
    printf("Scramble: ");
    FormulaPrint(scramble, stdout);
    putchar('\n');
    printf("Skeleton: ");
    FormulaPrint(skeleton, stdout);
    putchar('\n');

    if (parity) {
        puts("The cube has parity.");
        return;
    }
    if (corner_cycles == 0 && edge_cycles == 0) {
        puts("The cube is solved.");
        return;
    }
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

    FinderSolveStatus status;
    solve(args, &status);
    switch (status) {
        case SOLVE_SUCCESS:
            if (finder->solution_count == 0) {
                puts("No solution found.");
            }
            for (size_t i = 0; i < finder->solution_count; ++i) {
                printf("\nSolution #%lu:\n", i + 1);
                const Worker* solution = &finder->solution_list[i];
                for (size_t j = 0; j < solution->depth; ++j) {
                    const Insertion* insertion = &solution->solving_step[j];
                    const Formula* skeleton = &insertion->skeleton;
                    size_t insert_place = insertion->insert_place;
                    if (insert_place > 0) {
                        FormulaPrintRange(
                            skeleton,
                            0, insert_place,
                            stdout
                        );
                        putchar(' ');
                    }
                    printf("[@%lu]", j + 1);
                    if (insert_place < skeleton->length) {
                        putchar(' ');
                        FormulaPrintRange(
                            skeleton,
                            insert_place, skeleton->length,
                            stdout
                        );
                    }
                    printf("\nAt @%lu insert: ", j + 1);
                    FormulaPrint(insertion->insertion, stdout);
                    putchar('\n');
                }
                printf(
                    "Total moves: %lu,  %lu move%s cancelled.\n",
                    finder->fewest_moves,
                    solution->cancellation,
                    solution->cancellation == 1 ? "" : "s"
                );
                printf("Full solution: ");
                FormulaPrint(
                    &solution->solving_step[solution->depth].skeleton,
                    stdout
                );
                printf("\n");
            }
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
}


void JSONOutput(
    const Formula* scramble, const Formula* skeleton,
    bool parity,
    int corner_cycles, int edge_cycles,
    Finder* finder,
    SolvingFunction* solve, SolvingFunctionArgs* args
) {
    if (!parity) {
        FinderSolveStatus status;
        solve(args, &status);
    }

    JsonObject* object = json_object_new();

    char* scramble_string = Formula2String(scramble);
    json_object_set_string_member(object, "scramble", scramble_string);
    free(scramble_string);
    json_object_set_int_member(object, "scramble_moves", scramble->length);

    char* skeleton_string = Formula2String(skeleton);
    json_object_set_string_member(object, "skeleton", skeleton_string);
    free(skeleton_string);
    json_object_set_int_member(object, "skeleton_moves", skeleton->length);

    json_object_set_boolean_member(object, "parity", parity);
    if (!parity) {
        json_object_set_int_member(object, "corner_cycle_num", corner_cycles);
        json_object_set_int_member(object, "edge_cycle_num", edge_cycles);
        if (
            finder->solution_count
            || (corner_cycles == 0 && edge_cycles == 0)
        ) {
            json_object_set_int_member(
                object,
                "minimum_moves",
                finder->fewest_moves
            );
        } else {
            json_object_set_null_member(object, "minimum_moves");
        }

        JsonArray* solution_array = json_array_new();
        for (size_t i = 0; i < finder->solution_count; ++i) {
            Solution2JSON(&finder->solution_list[i], solution_array);
        }
        json_object_set_array_member(object, "solution", solution_array);
    }

    JsonNode* json = json_node_alloc();
    json_node_init_object(json, object);
    json_object_unref(object);

    PrintJson(json, stdout);
    json_node_unref(json);
}


void Solution2JSON(const Worker* solution, JsonArray* solution_array) {
    JsonObject* object = json_object_new();

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
            FormulaPrintRange(skeleton, 0, insert_place, skeleton_stream);
            fputc(' ', skeleton_stream);
        }
        fputc('@', skeleton_stream);
        if (insert_place < skeleton->length) {
            fputc(' ', skeleton_stream);
            FormulaPrintRange(
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

        char* insertion_string = Formula2String(insertion->insertion);
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
