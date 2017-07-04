#include <stdio.h>
#include <stdlib.h>
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
        FILE* algorithm_file = fopen(parsed_args->algfile_list[i], "r");
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
        printf("Scramble: ");
        FormulaPrint(&scramble, stdout);
        printf("\nSkeleton: ");
        FormulaPrint(&skeleton, stdout);
        printf("\n");

        Finder finder;
        FinderConstruct(&finder, map.size, algorithm_list, &scramble);
        FinderSolve(&finder, &skeleton);

        for (size_t i = 0; i < finder.solution_count; ++i) {
            printf("\nSolution #%lu:\n", i + 1);
            FinderWorker* solution = &finder.solution_list[i];
            for (size_t j = 0; j < solution->depth; ++j) {
                const Insertion* insertion = &solution->solving_step[j];
                const Formula* skeleton = &insertion->skeleton;
                size_t insert_place = insertion->insert_place;
                if (insert_place > 0) {
                    FormulaPrintRange(skeleton, 0, insert_place, stdout);
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
                printf("\nInsert at @%lu: ", j + 1);
                FormulaPrint(insertion->insertion, stdout);
                putchar('\n');
            }
            printf(
                "Total moves: %lu,  %lu move%s cancelled.\n",
                finder.fewest_moves,
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
