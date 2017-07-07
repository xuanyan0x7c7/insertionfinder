#include <stdio.h>
#include <stdlib.h>
#include <json-glib/json-glib.h>
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "utils.h"
#include "commands.h"


typedef void OutputFunction(const Formula*, const Formula*, bool, int, int);


static void StandardOutput(
    const Formula* scramble, const Formula* skeleton,
    bool parity, int corner_cycles, int edge_cycles
);

static void JSONOutput(
    const Formula* scramble, const Formula* skeleton,
    bool parity, int corner_cycles, int edge_cycles
);


bool Verify(const CliParser* parsed_args) {
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

        OutputFunction* print;
        bool parity;
        int corner_cycles;
        int edge_cycles;

        Cube cube = identity_cube;
        CubeTwistFormula(&cube, &scramble, true, true, false);
        CubeTwistFormula(&cube, &skeleton, true, true, false);
        if (CubeHasParity(&cube)) {
            parity = true;
        } else {
            parity = false;
            corner_cycles = CubeCornerCycles(&cube);
            edge_cycles = CubeEdgeCycles(&cube);
        }

        print = parsed_args->json ? JSONOutput : StandardOutput;
        print(&scramble, &skeleton, parity, corner_cycles, edge_cycles);

        if (input != stdin) {
            fclose(input);
        }
    } while (false);

    free(scramble_string);
    free(skeleton_string);
    return success;
}


void StandardOutput(
    const Formula* scramble, const Formula* skeleton,
    bool parity, int corner_cycles, int edge_cycles
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
}


void JSONOutput(
    const Formula* scramble, const Formula* skeleton,
    bool parity, int corner_cycles, int edge_cycles
) {
    JsonBuilder* builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "scramble");
    char* scramble_string = Formula2String(scramble);
    json_builder_add_string_value(builder, scramble_string);
    free(scramble_string);
    json_builder_set_member_name(builder, "skeleton");
    char* skeleton_string = Formula2String(skeleton);
    json_builder_add_string_value(builder, skeleton_string);
    free(skeleton_string);
    json_builder_set_member_name(builder, "parity");
    json_builder_add_boolean_value(builder, parity);
    if (!parity) {
        json_builder_set_member_name(builder, "corner_cycle_num");
        json_builder_add_int_value(builder, corner_cycles);
        json_builder_set_member_name(builder, "edge_cycle_num");
        json_builder_add_int_value(builder, edge_cycles);
    }
    json_builder_end_object(builder);
    JsonGenerator* generator = json_generator_new();
    JsonNode* root = json_builder_get_root(builder);
    json_generator_set_root(generator, root);
    gchar* string = json_generator_to_data(generator, NULL);
    json_node_free(root);
    g_object_unref(generator);
    g_object_unref(builder);
    printf("%s", string);
    free(string);
}
