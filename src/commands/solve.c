#include <stdio.h>
#include <stdlib.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
#include "../finder/finder.h"
#include "../formula/formula.h"
#include "utils.h"
#include "commands.h"


bool Solve(const CliParser* parsed_args) {
    size_t algorithm_count = 0;
    size_t algorithm_capacity = 1024;
    Algorithm** algorithm_list = (Algorithm**)malloc(
        algorithm_capacity * sizeof(Algorithm*)
    );
    for (size_t i = 0; i < parsed_args->algfile_count; ++i) {
        FILE* algorithm_file = fopen(parsed_args->algfile_list[i], "r");
        size_t size;
        fread(&size, sizeof(size_t), 1, algorithm_file);
        for (size_t j = 0; j < size; ++j) {
            if (algorithm_count == algorithm_capacity) {
                algorithm_list = (Algorithm**)realloc(
                    algorithm_list,
                    (algorithm_capacity <<= 1) * sizeof(Algorithm*)
                );
            }
            algorithm_list[algorithm_count++] = AlgorithmLoad(
                NULL,
                algorithm_file
            );
        }
        fclose(algorithm_file);
    }

    const char* filepath = parsed_args->file_list[0];
    char* scramble_string = NULL;
    char* partial_solve_string = NULL;
    FILE* input = filepath ? fopen(filepath, "r") : stdin;
    if (!input) {
        fprintf(stderr, "Fail to open file: %s\n", filepath);
        return false;
    }

    bool success = true;
    do {
        if (!(
            (scramble_string = GetLine(input))
            && (partial_solve_string = GetLine(input))
        )) {
            fputs("Error input", stderr);
            success = false;
            break;
        }

        Formula scramble;
        Formula partial_solution;
        if (!FormulaConstruct(&scramble, scramble_string)) {
            fprintf(stderr, "Invalid scramble sequence: %s\n", scramble_string);
            success = false;
            break;
        }
        if (!FormulaConstruct(&partial_solution, partial_solve_string)) {
            fprintf(
                stderr,
                "Invalid partial solve sequence: %s\n",
                partial_solve_string
            );
            success = false;
            break;
        }
        printf("Scramble: ");
        FormulaPrint(&scramble, stdout);
        printf("\nPartial Solution: ");
        FormulaPrint(&partial_solution, stdout);
        printf("\n");

        Finder finder;
        FinderConstruct(&finder, algorithm_count, algorithm_list, &scramble);
        FinderSolve(&finder, &partial_solution);

        for (size_t i = 0; i < finder.solution_count; ++i) {
            printf("\nSolution #%lu:\n", i + 1);
            const FinderWorker* solution = &finder.solution_list[i];
            size_t cancelled_moves = solution->solving_step[
                0
            ].partial_solution.length;
            for (size_t j = 0; j < solution->depth; ++j) {
                const Insertion* insertion = &solution->solving_step[j];
                const Formula* partial_solution = &insertion->partial_solution;
                cancelled_moves += insertion->insertion->length;
                size_t insert_place = insertion->insert_place;
                if (insert_place > 0) {
                    FormulaPrintRange(
                        partial_solution,
                        0, insert_place,
                        stdout
                    );
                    putchar(' ');
                }
                printf("[@%lu]", j + 1);
                if (insert_place < partial_solution->length) {
                    putchar(' ');
                    FormulaPrintRange(
                        partial_solution,
                        insert_place, partial_solution->length,
                        stdout
                    );
                }
                printf("\nInsert at [@%lu]: ", j + 1);
                FormulaPrint(insertion->insertion, stdout);
                putchar('\n');
            }
            cancelled_moves -= solution->solving_step[
                solution->depth
            ].partial_solution.length;
            printf(
                "Fewest Moves: %lu,  %lu move%s cancelled.\n",
                finder.fewest_moves,
                cancelled_moves,
                cancelled_moves == 1 ? "" : "s"
            );
            printf("Full solution: ");
            FormulaPrint(
                &solution->solving_step[solution->depth].partial_solution,
                stdout
            );
            printf("\n");
        }

        if (input != stdin) {
            fclose(input);
        }
    } while (false);

    for (size_t i = 0; i < algorithm_count; ++i) {
        AlgorithmDestroy(algorithm_list[i]);
        free(algorithm_list[i]);
    }
    free(algorithm_list);
    free(scramble_string);
    free(partial_solve_string);
    return success;
}
