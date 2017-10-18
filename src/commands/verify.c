#include <stdio.h>
#include <stdlib.h>
#include <json-glib/json-glib.h>
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "utils.h"
#include "commands.h"


typedef void OutputFunction(const Formula*, const Formula*, bool, int, int);


static void standard_output(
    const Formula* scramble, const Formula* skeleton,
    bool parity, int corner_cycles, int edge_cycles
);

static void json_output(
    const Formula* scramble, const Formula* skeleton,
    bool parity, int corner_cycles, int edge_cycles
);


bool verify(const CliParser* parsed_args) {
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

        OutputFunction* print =
            parsed_args->json ? json_output : standard_output;
        print(&scramble, &skeleton, parity, corner_cycles, edge_cycles);

        if (input != stdin) {
            fclose(input);
        }
    } while (false);

    free(scramble_string);
    free(skeleton_string);
    return success;
}


void standard_output(
    const Formula* scramble, const Formula* skeleton,
    bool parity, int corner_cycles, int edge_cycles
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
}


void json_output(
    const Formula* scramble, const Formula* skeleton,
    bool parity, int corner_cycles, int edge_cycles
) {
    JsonObject* object = json_object_new();

    char* scramble_string = formula_to_string(scramble);
    json_object_set_string_member(object, "scramble", scramble_string);
    free(scramble_string);

    char* skeleton_string = formula_to_string(skeleton);
    json_object_set_string_member(object, "skeleton", skeleton_string);
    free(skeleton_string);

    json_object_set_boolean_member(object, "parity", parity);
    json_object_set_int_member(object, "corner_cycle_num", corner_cycles);
    json_object_set_int_member(object, "edge_cycle_num", edge_cycles);

    JsonNode* json = json_node_alloc();
    json_node_init_object(json, object);
    json_object_unref(object);

    print_json(json, stdout);
    json_node_unref(json);
}
