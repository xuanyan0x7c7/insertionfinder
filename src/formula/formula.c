#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "../data-structure/linked-list.h"
#include "../utils/memory.h"
#include "../utils/io.h"
#include "formula.h"
#include "utils.h"


#define array_length(x) sizeof(x) / sizeof(x[0])


static const char* twist_str[] = {
    "", "U", "U2", "U'",
    "", "D", "D2", "D'",
    "", "R", "R2", "R'",
    "", "L", "L2", "L'",
    "", "F", "F2", "F'",
    "", "B", "B2", "B'"
};

static regex_t moves_regex;


static int compare(const void* p, const void* q);
static void formula_append(Formula* formula, int move);


void formula_init() {
    int status = regcomp(
        &moves_regex,
        "\\s*(([UDRLFB]|2?[UDRLFB]w)[2']?|[xyz][2']?|\\[[udrlfb][2']?\\])\\s*",
        REG_EXTENDED
    );
    if (status) {
        char message[100];
        regerror(status, &moves_regex, message, 99);
        fprintf(stderr, "%s\n", message);
        exit(EXIT_FAILURE);
    }
}


bool formula_construct(Formula* formula, const char* string) {
    typedef struct Pattern Pattern;
    struct Pattern {
        const char* twist_string[7];
        int transform[3];
        const int additional_move;
    };

    const Pattern pattern_table[] = {
        {{"x", "[r]", "[l']", NULL}, {4, 2, 1}, -1},
        {{"x2", "[r2]", "[l2]", NULL}, {1, 2, 5}, -1},
        {{"x'", "[r']", "[l]", NULL}, {5, 2, 0}, -1},
        {{"y", "[u]", "[d']", NULL}, {0, 5, 2}, -1},
        {{"y2", "[u2]", "[d2]", NULL}, {0, 3, 5}, -1},
        {{"y'", "[u']", "[d]", NULL}, {0, 4, 3}, -1},
        {{"z", "[f]", "[b']", NULL}, {3, 0, 4}, -1},
        {{"z2", "[f2]", "[b2]", NULL}, {1, 3, 4}, -1},
        {{"z'", "[f']", "[b]", NULL}, {2, 1, 4}, -1},
        {{"Uw", NULL}, {0, 5, 2}, TWIST_D},
        {{"Uw2", NULL}, {0, 3, 5}, TWIST_D2},
        {{"Uw'", NULL}, {0, 4, 3}, TWIST_D3},
        {{"Dw", NULL}, {0, 4, 3}, TWIST_U},
        {{"Dw2", NULL}, {0, 3, 5}, TWIST_U2},
        {{"Dw'", NULL}, {0, 5, 2}, TWIST_U3},
        {{"Rw", NULL}, {4, 2, 1}, TWIST_L},
        {{"Rw2", NULL}, {1, 2, 5}, TWIST_L2},
        {{"Rw'", NULL}, {5, 2, 0}, TWIST_L3},
        {{"Lw", NULL}, {5, 2, 0}, TWIST_R},
        {{"Lw2", NULL}, {1, 2, 5}, TWIST_R2},
        {{"Lw'", NULL}, {4, 2, 1}, TWIST_R3},
        {{"Fw", NULL}, {3, 0, 4}, TWIST_B},
        {{"Fw2", NULL}, {1, 3, 4}, TWIST_B2},
        {{"Fw'", NULL}, {2, 1, 4}, TWIST_B3},
        {{"Bw", NULL}, {2, 1, 4}, TWIST_F},
        {{"Bw2", NULL}, {1, 3, 4}, TWIST_F2},
        {{"Bw'", NULL}, {3, 0, 4}, TWIST_F3}
    };

    formula->length = 0;
    formula->capacity = 64;
    formula->move = MALLOC(int, formula->capacity);
    if (!string || string[0] == '\0') {
        return true;
    }

    int transform[3] = {0, 2, 4};

    const char* buffer = string;
    regmatch_t pmatch[3];
    while (*buffer) {
        if (regexec(&moves_regex, buffer, 3, pmatch, 0) == REG_NOMATCH) {
            formula_destroy(formula);
            return false;
        }
        const regmatch_t* full_match = &pmatch[0];
        if (full_match->rm_so) {
            formula_destroy(formula);
            return false;
        }
        const regmatch_t* match = &pmatch[1];
        size_t length = match->rm_eo - match->rm_so;
        const char* begin = &buffer[match->rm_so];
        if (*begin == '2') {
            ++begin;
            --length;
        }
        char move_string[length + 1];
        strncpy(move_string, begin, length);
        move_string[length] = '\0';

        bool pattern_found = false;
        for (size_t i = 0; i < array_length(pattern_table); ++i) {
            const Pattern* pattern_item = &pattern_table[i];
            const char* const* twist_string = pattern_item->twist_string;
            for (const char* const* p = twist_string; *p; ++p) {
                if (strcmp(move_string, *p) == 0) {
                    pattern_found = true;
                    break;
                }
            }
            if (pattern_found) {
                const int* pattern_transform = pattern_item->transform;
                int additional_move = pattern_item->additional_move;
                if (additional_move != -1) {
                    formula_append(
                        formula,
                        transform[additional_move >> 3] << 2
                            ^ (additional_move & 7)
                    );
                }
                int new_transform[3];
                for (int j = 0; j < 3; ++j) {
                    int temp = pattern_transform[j];
                    new_transform[j] = transform[temp >> 1] ^ (temp & 1);
                }
                memcpy(transform, new_transform, 3 * sizeof(int));
                break;
            }
        }
        if (!pattern_found) {
            for (size_t j = 0; j < array_length(twist_str); ++j) {
                if (strcmp(move_string, twist_str[j])) {
                    continue;
                }
                formula_append(formula, transform[j >> 3] << 2 ^ (j & 7));
                break;
            }
        }
        buffer += match->rm_eo;
    }

    formula_cancel_moves(formula);
    return true;
}

void formula_destroy(Formula* formula) {
    free(formula->move);
}


void formula_save(const Formula* formula, FILE* stream) {
    fwrite(&formula->length, sizeof(size_t), 1, stream);
    int8_t move[formula->length];
    for (size_t i = 0; i < formula->length; ++i) {
        move[i] = formula->move[i];
    }
    fwrite(move, sizeof(int8_t), formula->length, stream);
}

bool formula_load(Formula* formula, FILE* stream) {
    size_t length;
    if (!safe_read(&length, sizeof(size_t), 1, stream)) {
        return false;
    }
    formula->length = length;
    formula->capacity = formula_get_min_capacity(length);
    formula->move = MALLOC(int, formula->capacity);
    int8_t move[length];
    if (!safe_read(move, sizeof(int8_t), length, stream)) {
        return false;
    }
    for (size_t i = 0; i < length; ++i) {
        formula->move[i] = move[i];
    }
    formula->begin_mask = formula_move_mask(inverse_move_table[move[0]]);
    if (length > 1 && move[0] >> 3 == move[1] >> 3) {
        formula->begin_mask |= formula_move_mask(inverse_move_table[move[1]]);
    }
    formula->end_mask = formula_move_mask(inverse_move_table[move[length - 1]]);
    if (length > 1 && move[length - 1] >> 3 == move[length - 2] >> 3) {
        formula->end_mask |= formula_move_mask(inverse_move_table[move[length - 2]]);
    }
    if (length > 2) {
        uint32_t set_up_mask = (formula->begin_mask & formula->end_mask) >> 24;
        formula->set_up_mask = 0;
        for (int i = 0; i < 6; ++i) {
            if (set_up_mask & 1 << i) {
                formula->set_up_mask |= 0xe << (i << 2);
            }
        }
        formula->set_up_mask &= formula->end_mask;
    } else {
        formula->set_up_mask = 0;
    }
    return true;
}

void formula_duplicate(Formula* formula, const Formula* source) {
    size_t length = source->length;
    formula->length = length;
    formula->capacity = formula_get_min_capacity(length);
    formula->move = MALLOC(int, formula->capacity);
    memcpy(formula->move, source->move, length * sizeof(int));
}


void formula_print(const Formula* formula, FILE* stream) {
    if (formula->length == 0) {
        return;
    }
    const int* begin = formula->move;
    const int* end = begin + formula->length;
    fputs(twist_str[*begin], stream);
    for (const int* p = begin; ++p < end;) {
        fprintf(stream, " %s", twist_str[*p]);
    }
}

void formula_print_range(
    const Formula* formula,
    size_t begin, size_t end,
    FILE* stream
) {
    if (begin == end) {
        return;
    }
    const int* p_begin = formula->move + begin;
    const int* p_end = formula->move + end;
    fputs(twist_str[*p_begin], stream);
    for (const int* p = p_begin; ++p < p_end;) {
        fprintf(stream, " %s", twist_str[*p]);
    }
}


char* formula_to_string(const Formula* formula) {
    char* buffer;
    size_t size;
    FILE* stream = open_memstream(&buffer, &size);
    formula_print(formula, stream);
    fclose(stream);
    return buffer;
}


bool formula_swappable(const Formula* formula, size_t index) {
    return index > 0 && index < formula->length
        && formula->move[index - 1] >> 3 == formula->move[index] >> 3;
}

void formula_swap_adjacent(Formula* formula, size_t index) {
    int temp = formula->move[index - 1];
    formula->move[index - 1] = formula->move[index];
    formula->move[index] = temp;
}

void formula_normalize(Formula* formula) {
    for (size_t i = 1; i < formula->length; ++i) {
        if (
            formula->move[i - 1] >> 3 == formula->move[i] >> 3
            && formula->move[i - 1] > formula->move[i]
        ) {
            formula_swap_adjacent(formula, i++);
        }
    }
}

int formula_compare(const Formula* f1, const Formula* f2) {
    if (f1->length != f2->length) {
        return f1->length - f2->length;
    }
    for (size_t i = 0; i < f1->length; ++i) {
        if (f1->move[i] != f2->move[i]) {
            return f1->move[i] - f2->move[i];
        }
    }
    return 0;
}


size_t formula_generate_isomorphisms(const Formula* formula, Formula* result) {
    static const int transform_table[][3] = {
        {0, 2, 4}, {0, 4, 3}, {0, 3, 5}, {0, 5, 2},
        {1, 2, 5}, {1, 4, 2}, {1, 3, 4}, {1, 5, 3},
        {4, 0, 2}, {3, 0, 4}, {5, 0, 3}, {2, 0, 5},
        {4, 1, 3}, {2, 1, 4}, {5, 1, 2}, {3, 1, 5},
        {4, 3, 0}, {2, 4, 0}, {5, 2, 0}, {3, 5, 0},
        {4, 2, 1}, {3, 4, 1}, {5, 3, 1}, {2, 5, 1}
    };
    size_t length = formula->length;
    for (size_t i = 0; i < 96; ++i) {
        result[i].length = length;
        result[i].capacity = formula_get_min_capacity(length);
        result[i].move = MALLOC(int, result[i].capacity);
    }
    for (size_t i = 0; i < 24; ++i) {
        int* move_list = result[i].move;
        int* reversed_list = result[i + 24].move;
        int* flipped_list = result[i + 48].move;
        int* reversed_flipped_list = result[i + 72].move;
        const int* table = transform_table[i];
        for (size_t j = 0; j < length; ++j) {
            int inversed_index = length - 1 - j;
            int move = formula->move[j];
            int result_move = table[move >> 3] << 2 ^ (move & 7);
            move_list[j] = result_move;
            reversed_list[inversed_index] = inverse_move_table[result_move];
            flipped_list[j] = result_move < 16
                ? inverse_move_table[result_move]
                : 40 - result_move;
            reversed_flipped_list[inversed_index] = inverse_move_table[
                flipped_list[j]
            ];
        }
        formula_normalize(&result[i]);
        formula_normalize(&result[i + 24]);
        formula_normalize(&result[i + 48]);
        formula_normalize(&result[i + 72]);
    }
    qsort(result, 96, sizeof(Formula), compare);
    Formula* p = result;
    Formula* q = result + 1;
    while (true) {
        while (q != result + 96 && formula_compare(p, q) == 0) {
            formula_destroy(q++);
        }
        if (q == result + 96) {
            break;
        }
        if (++p != q) {
            memcpy(p, q, sizeof(Formula));
        }
        ++q;
    }
    return p - result + 1;
}


int compare(const void* p, const void* q) {
    return formula_compare((const Formula*)p, (const Formula*)q);
}


void formula_append(Formula* formula, int move) {
    if (formula->length == formula->capacity) {
        REALLOC(formula->move, int, formula->capacity <<= 1);
    }
    formula->move[formula->length++] = move;
}
