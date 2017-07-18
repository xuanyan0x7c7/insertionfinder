#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/memory.h"
#include "formula.h"
#include "utils.h"


size_t FormulaCancelMoves(Formula* formula) {
    int* begin = formula->move;
    int* end = begin + formula->length;
    int* p = begin - 1;
    int* needle = end;

    for (const int* q = begin; q < end; ++q) {
        if (p < begin || *p >> 3 != *q >> 3) {
            *++p = *q;
        } else if (*p >> 2 != *q >> 2) {
            if (p > begin && *(p - 1) >> 3 == *p >> 3) {
                if (needle > p) {
                    needle = p;
                }
                int orientation = (*(p - 1) + *q) & 3;
                if (orientation == 0) {
                    *(p - 1) = *p;
                    --p;
                } else {
                    *(p - 1) = (*(p - 1) & ~3) | orientation;
                }
            } else {
                *++p = *q;
            }
        } else {
            int orientation = (*p + *q) & 3;
            if (orientation == 0) {
                --p;
            } else {
                *p = (*p & ~3) | orientation;
            }
            if (needle > p + 1) {
                needle = p + 1;
            }
        }
    }

    formula->length = p + 1 - begin;
    return needle - begin;
}


void FormulaGetInsertPlaceMask(
    const Formula* formula,
    size_t insert_place,
    uint32_t* mask
) {
    const int* move = formula->move;
    if (insert_place == 0) {
        mask[0] = 0;
    } else {
        mask[0] = MoveMask(move[insert_place - 1]);
        if (
            insert_place > 1
            && move[insert_place - 1] >> 3 == move[insert_place - 2] >> 3
        ) {
            mask[0] |= MoveMask(move[insert_place - 2]);
        }
    }
    if (insert_place == formula->length) {
        mask[1] = 0;
    } else {
        mask[1] = MoveMask(move[insert_place]);
        if (
            insert_place + 1 < formula->length
            && move[insert_place] >> 3 == move[insert_place + 1] >> 3
        ) {
            mask[1] |= MoveMask(move[insert_place + 1]);
        }
    }
}


size_t FormulaInsert(
    const Formula* formula,
    size_t insert_place,
    const Formula* insertion,
    Formula* result
) {
    result->length = formula->length + insertion->length;
    result->capacity = 32;
    while ((result->capacity <<= 1) < result->length);
    result->move = MALLOC(int, result->capacity);
    memcpy(result->move, formula->move, insert_place * sizeof(int));
    memcpy(
        result->move + insert_place,
        insertion->move,
        insertion->length * sizeof(int)
    );
    memcpy(
        result->move + insert_place + insertion->length,
        formula->move + insert_place,
        (formula->length - insert_place) * sizeof(int)
    );
    size_t place = FormulaCancelMoves(result);
    return place <= insert_place ? place : insert_place + 1;
}

bool FormulaInsertIsWorthy(
    const Formula* formula,
    size_t insert_place,
    const Formula* insertion,
    const uint32_t* insert_place_mask,
    size_t fewest_moves
) {
    size_t cancellation = 0;
    uint32_t begin_mask = insert_place_mask[0] & insertion->begin_mask;
    if (begin_mask) {
        if (begin_mask & 0xffffff) {
            return false;
        }
        uint32_t high_mask = begin_mask >>= 24;
        cancellation += (high_mask & (high_mask - 1)) ? 2 : 1;
    }
    uint32_t end_mask = insert_place_mask[1] & insertion->end_mask;
    if (end_mask) {
        if (end_mask & insertion->set_up_mask) {
            return false;
        }
        uint32_t low_mask = end_mask & 0xffffff;
        uint32_t high_mask = end_mask >> 24;
        if (high_mask & (high_mask - 1)) {
            if (low_mask == 0) {
                cancellation += 2;
            } else if (low_mask & (low_mask - 1)) {
                cancellation += (formula->length - insert_place) << 1;
            } else {
                cancellation += 3;
            }
        } else if (low_mask) {
            cancellation += (formula->length - insert_place) << 1;
        } else {
            ++cancellation;
        }
    }
    return fewest_moves == SIZE_MAX
        || formula->length + insertion->length <= fewest_moves + cancellation;
}
