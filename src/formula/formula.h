#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum {
    TWIST_U0, TWIST_U, TWIST_U2, TWIST_U3,
    TWIST_D0, TWIST_D, TWIST_D2, TWIST_D3,
    TWIST_R0, TWIST_R, TWIST_R2, TWIST_R3,
    TWIST_L0, TWIST_L, TWIST_L2, TWIST_L3,
    TWIST_F0, TWIST_F, TWIST_F2, TWIST_F3,
    TWIST_B0, TWIST_B, TWIST_B2, TWIST_B3
};

static const int inverse_move_table[] = {
    TWIST_U0, TWIST_U3, TWIST_U2, TWIST_U,
    TWIST_D0, TWIST_D3, TWIST_D2, TWIST_D,
    TWIST_R0, TWIST_R3, TWIST_R2, TWIST_R,
    TWIST_L0, TWIST_L3, TWIST_L2, TWIST_L,
    TWIST_F0, TWIST_F3, TWIST_F2, TWIST_F,
    TWIST_B0, TWIST_B3, TWIST_B2, TWIST_B
};

typedef struct Formula Formula;
struct Formula {
    size_t length;
    size_t capacity;
    int* move;
    uint32_t begin_mask;
    uint32_t end_mask;
    uint32_t set_up_mask;
};

void formula_init();

bool formula_construct(Formula* formula, const char* string);
void formula_destroy(Formula* formula);

void formula_save(const Formula* formula, FILE* stream);
bool formula_load(Formula* formula, FILE* stream);
void formula_duplicate(Formula* formula, const Formula* source);

void formula_print(const Formula* formula, FILE* stream);
void formula_print_range(
    const Formula* formula,
    size_t begin, size_t end,
    FILE* stream
);

char* formula_to_string(const Formula* formula);

size_t formula_cancel_moves(Formula* formula);

void formula_get_insert_place_mask(
    const Formula* formula,
    size_t insert_place,
    uint32_t* mask
);

size_t formula_insert(
    const Formula* formula,
    size_t insert_place,
    const Formula* insertion,
    Formula* result
);
bool formula_insert_is_worthy(
    const Formula* formula,
    size_t insert_place,
    const Formula* insertion,
    const uint32_t* insert_place_mask,
    size_t fewest_moves
);

bool formula_swappable(const Formula* formula, size_t index);
void formula_swap_adjacent(Formula* formula, size_t index);
void formula_normalize(Formula* formula);
int formula_compare(const Formula* f1, const Formula* f2);

size_t formula_generate_isomorphisms(const Formula* formula, Formula* result);
