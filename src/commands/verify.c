#include <stdio.h>
#include <stdlib.h>
#include "../cube/cube.h"
#include "../formula/formula.h"
#include "utils.h"
#include "commands.h"


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
        printf("Scramble: ");
        FormulaPrint(&scramble, stdout);
        putchar('\n');
        printf("Skeleton: ");
        FormulaPrint(&skeleton, stdout);
        putchar('\n');

        Cube cube;
        CubeConstruct(&cube);
        CubeTwistFormula(&cube, &scramble, true, true, false);
        CubeTwistFormula(&cube, &skeleton, true, true, false);
        if (CubeHasParity(&cube)) {
            puts("The cube has parity.");
        } else {
            int corner_cycles = CubeCornerCycles(&cube);
            int edge_cycles = CubeEdgeCycles(&cube);
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

        if (input != stdin) {
            fclose(input);
        }
    } while (false);
    free(scramble_string);
    free(skeleton_string);
    return success;
}
