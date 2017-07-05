#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "../data-structure/linked-list.h"
#include "formula.h"


#define ArrayLength(x) sizeof(x) / sizeof(x[0])


static const char twist_str[][3] = {
    "", "U", "U2", "U'",
    "", "D", "D2", "D'",
    "", "R", "R2", "R'",
    "", "L", "L2", "L'",
    "", "F", "F2", "F'",
    "", "B", "B2", "B'"
};

static regex_t moves_regex;


static void CycleReplace(char* c, const char* pattern);
static int FormulaCompareGeneric(const void* p, const void* q);


void FormulaInit() {
    int status = regcomp(
        &moves_regex,
        "\\s*([UDRLFB]w?2?'?|[xyz]2?'?|\\[[udrlfb]2?'?\\])\\s*",
        REG_EXTENDED
    );
    if (status) {
        char message[100];
        regerror(status, &moves_regex, message, 99);
        fprintf(stderr, "%s\n", message);
        exit(EXIT_FAILURE);
    }
}


bool FormulaConstruct(Formula* formula, const char* string) {
    typedef struct BlockMovePattern BlockMovePattern;
    struct BlockMovePattern {
        const char* pattern;
        const char* rotation;
        const char* twist;
    };
    static const BlockMovePattern block_move_table[] = {
        {"Uw", "y", "D"}, {"Uw2", "y2", "D2"}, {"Uw'", "y'", "D'"},
        {"Dw", "y'", "U"}, {"Dw2", "y2", "U2"}, {"Dw'", "y", "U'"},
        {"Rw", "x", "L"}, {"Rw2", "x2", "L2"}, {"Rw'", "x'", "L'"},
        {"Lw", "x'", "R"}, {"Lw2", "x2", "R2"}, {"Lw'", "x", "R'"},
        {"Fw", "z", "B"}, {"Fw2", "z2", "B2"}, {"Fw'", "z'", "B'"},
        {"Bw", "z'", "F"}, {"Bw2", "z2", "F2"}, {"Bw'", "z", "F'"}
    };

    typedef char Patterns[3][5];
    static const Patterns rotation_replacement[][2] = {
        {{"x", "[r]", "[l']"}, {"UFDB", ""}},
        {{"x2", "[r2]", "[l2]"}, {"UD", "FB", ""}},
        {{"x'", "[r']", "[l]"}, {"UBDF", ""}},
        {{"y", "[u]", "[d']"}, {"RBLF", ""}},
        {{"y2", "[u2]", "[d2]"}, {"RL", "FB", ""}},
        {{"y'", "[u']", "[d]"}, {"RFLB", ""}},
        {{"z", "[f]", "[b']"}, {"ULDR", ""}},
        {{"z2", "[f2]", "[b2]"}, {"UD", "RL", ""}},
        {{"z'", "[f']", "[b]"}, {"URDL", ""}}
    };

    if (!string) {
        formula->length = 0;
        formula->capacity = 64;
        formula->move = (int*)malloc(formula->capacity * sizeof(int));
        return true;
    }

    LinkedList procedure;
    LinkedListConstruct(&procedure, free);

    const char* buffer = string;
    regmatch_t pmatch[2];
    while (buffer && regexec(&moves_regex, buffer, 2, pmatch, 0) == 0) {
        regmatch_t* full_match = &pmatch[0];
        if (full_match->rm_so) {
            FormulaDestroy(formula);
            return false;
        }
        regmatch_t* match = &pmatch[1];
        size_t length = match->rm_eo - match->rm_so;
        char* move_string = (char*)malloc((length + 1) * sizeof(char));
        strncpy(move_string, buffer + match->rm_so, length);
        move_string[length] = '\0';
        char* position = strstr(move_string, "2'");
        if (position) {
            while (*++position) {
                *position = *(position + 1);
            }
        }
        LinkedListInsertBefore(procedure.tail, move_string);
        buffer += match->rm_eo;
    }

    LinkedListNode* node = procedure.head;
    while ((node = node->next) != procedure.tail) {
        const char* move_string = (char*)node->data;
        for (size_t i = 0; i < ArrayLength(block_move_table); ++i) {
            const BlockMovePattern* table = &block_move_table[i];
            if (strcmp(move_string, table->pattern) == 0) {
                LinkedListSetItem(
                    &procedure,
                    node,
                    strdup(table->rotation)
                );
                LinkedListInsertBefore(node, strdup(table->twist));
                break;
            }
        }
    }

    for (
        const LinkedListNode* pointer = procedure.tail;
        (pointer = pointer->prev) != procedure.head;
    ) {
        for (size_t i = 0; i < ArrayLength(rotation_replacement); ++i) {
            const Patterns* matches = &rotation_replacement[i][0];
            const Patterns* patterns = &rotation_replacement[i][1];
            bool found = false;
            for (size_t j = 0; j < 3; ++j) {
                if (strcmp((char*)pointer->data, (*matches)[j]) == 0) {
                    found = true;
                    break;
                }
            }
            if (found) {
                for (
                    const LinkedListNode* node = pointer;
                    (node = node->next) != procedure.tail;
                ) {
                    for (size_t j = 0; ; ++j) {
                        const char* pattern = (*patterns)[j];
                        if (pattern[0]) {
                            CycleReplace((char*)node->data, pattern);
                        } else {
                            break;
                        }
                    }
                }
                pointer = pointer->next;
                LinkedListRemove(&procedure, pointer->prev);
                break;
            }
        }
    }

    formula->length = 0;
    formula->capacity = 64;
    formula->move = (int*)malloc(formula->capacity * sizeof(int));
    for (
        const LinkedListNode* node = procedure.head;
        (node = node->next) != procedure.tail;
    ) {
        for (size_t i = 0; i < ArrayLength(twist_str); ++i) {
            if (strcmp((char*)node->data, twist_str[i]) == 0) {
                if (formula->length == formula->capacity) {
                    formula->move = (int*)realloc(
                        formula->move,
                        (formula->capacity <<= 1) * sizeof(int)
                    );
                }
                formula->move[formula->length++] = i;
                break;
            }
        }
    }
    LinkedListDestroy(&procedure);
    FormulaCancelMoves(formula);
    return formula;
}

void FormulaDestroy(Formula* formula) {
    free(formula->move);
}


void FormulaSave(const Formula* formula, FILE* stream) {
    fwrite(&formula->length, sizeof(size_t), 1, stream);
    int8_t compressed[formula->length];
    for (size_t i = 0; i < formula->length; ++i) {
        compressed[i] = formula->move[i];
    }
    fwrite(compressed, sizeof(int8_t), formula->length, stream);
}

void FormulaLoad(Formula* formula, FILE* stream) {
    size_t length;
    fread(&length, sizeof(size_t), 1, stream);
    formula->length = length;
    formula->capacity = 32;
    while ((formula->capacity <<= 1) < length);
    formula->move = (int*)malloc(formula->capacity * sizeof(int));
    int8_t compressed[length];
    fread(compressed, sizeof(int8_t), length, stream);
    for (size_t i = 0; i < length; ++i) {
        formula->move[i] = compressed[i];
    }
}

void FormulaDuplicate(Formula* formula, const Formula* source) {
    size_t length = source->length;
    formula->length = length;
    formula->capacity = 32;
    while ((formula->capacity <<= 1) < length);
    formula->move = (int*)malloc(formula->capacity * sizeof(int));
    memcpy(formula->move, source->move, length * sizeof(int));
}


void FormulaPrint(const Formula* formula, FILE* stream) {
    if (formula->length == 0) {
        return;
    }
    fprintf(stream, "%s", twist_str[formula->move[0]]);
    for (size_t i = 1; i < formula->length; ++i) {
        fprintf(stream, " %s", twist_str[formula->move[i]]);
    }
}

void FormulaPrintRange(
    const Formula* formula,
    size_t begin, size_t end,
    FILE* stream
) {
    if (begin == end) {
        return;
    }
    fprintf(stream, "%s", twist_str[formula->move[begin]]);
    for (size_t i = begin; ++i < end;) {
        fprintf(stream, " %s", twist_str[formula->move[i]]);
    }
}


size_t FormulaCancelMoves(Formula* formula) {
    int x = 0;
    int y = -1;
    int length = formula->length;
    int index = length + 1;
    int* move = formula->move;

    while (x < length) {
        if (y < 0 || move[x] >> 3 != move[y] >> 3) {
            move[++y] = move[x++];
        } else if (move[x] >> 2 != move[y] >> 2) {
            if (y > 0 && move[y - 1] >> 3 == move[y] >> 3) {
                int orientation = (move[y - 1] + move[x++]) & 3;
                if (index > y) {
                    index = y;
                }
                if (orientation == 0) {
                    move[y - 1] = move[y];
                    --y;
                } else {
                    move[y - 1] = (move[y - 1] & ~3) | orientation;
                }
            } else {
                move[++y] = move[x++];
            }
        } else {
            int orientation = (move[y] + move[x++]) & 3;
            if (orientation == 0) {
                --y;
            } else {
                move[y] = (move[y] & ~3) | orientation;
            }
            if (index > y + 1) {
                index = y + 1;
            }
        }
    }

    formula->length = y + 1;
    return index;
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
    result->move = (int*)malloc(result->capacity * sizeof(int));
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
    const Formula* insertion
) {
    if (insert_place == 0) {
        return true;
    }
    size_t l1 = formula->length;
    size_t l2 = insertion->length;
    const int* f1 = formula->move;
    const int* f2 = insertion->move;
    const int* x = &f1[insert_place - 1];
    const int* y = f2;
    if (inverse_move_table[*x] == *y) {
        return false;
    }
    if (*x >> 3 == *y >> 3) {
        if (l1 > 1 && inverse_move_table[*(x - 1)] == *y) {
            return false;
        }
        if (l2 > 1 && inverse_move_table[*x] == *(y + 1)) {
            return false;
        }
    }
    if (l2 > 1) {
        const int* z = &f2[l2 - 1];
        if (*y >> 2 == *z >> 2 && inverse_move_table[*z] == f1[insert_place]) {
            return false;
        }
    }
    return true;
}

bool FormulaInsertFinalIsWorthy(
    const Formula* formula,
    size_t insert_place,
    const Formula* insertion,
    size_t moves_to_cancel
) {
    size_t l1 = formula->length;
    size_t l2 = insertion->length;
    const int* f1 = formula->move;
    const int* f2 = insertion->move;
    size_t sum = 0;
    if (insert_place > 0) {
        const int* x = &f1[insert_place - 1];
        const int* y = f2;
        if (*x >> 2 == *y >> 2) {
            if ((*x + *y) & 3) {
                ++sum;
                if (insert_place > 1 && *(x - 1) >> 3 == *x >> 3) {
                    if (l2 == 1) {
                        sum += 2;
                    } else if (*(y + 1) >> 3 == *y >> 3) {
                        if ((*(x - 1) + *(y + 1)) & 3) {
                            ++sum;
                        } else {
                            return false;
                        }
                    }
                }
            } else {
                return false;
            }
        } else if (*x >> 3 == *y >> 3) {
            sum += insert_place << 1;
            if (l1 > 1 && inverse_move_table[*(x - 1)] == *y) {
                return false;
            }
            if (l2 > 1 && inverse_move_table[*x] == *(y + 1)) {
                return false;
            }
        }
    }
    if (insert_place < l1) {
        const int* x = &f1[insert_place];
        const int* y = &f2[l2 - 1];
        if (*x >> 2 == *y >> 2) {
            if ((*x + *y) & 3) {
                ++sum;
                if (insert_place < l1 - 1 && *(x + 1) >> 3 == *x >> 3) {
                    if (l2 == 1) {
                        sum += 2;
                    } else if (*(y - 1) >> 3 == *y >> 3) {
                        if ((*(x + 1) + *(y - 1)) & 3) {
                            ++sum;
                        } else {
                            sum += 2;
                        }
                    }
                }
            } else {
                const int* z = f2;
                if (l2 > 1 && *x >> 3 == *z >> 3) {
                    if (*x >> 2 == *z >> 2 || *x >> 2 == *(z + 1) >> 2) {
                        return false;
                    }
                }
                sum += (l1 - insert_place) << 1;
            }
        } else if (*x >> 3 == *y >> 3) {
            sum += (l1 - insert_place) << 1;
        }
    }
    return sum >= moves_to_cancel;
}


bool FormulaSwappable(const Formula* formula, size_t index) {
    return index > 0 && index < formula->length
        && formula->move[index - 1] >> 3 == formula->move[index] >> 3;
}

void FormulaSwapAdjacent(Formula* formula, size_t index) {
    int temp = formula->move[index - 1];
    formula->move[index - 1] = formula->move[index];
    formula->move[index] = temp;
}

void FormulaNormalize(Formula* formula) {
    for (size_t i = 1; i < formula->length; ++i) {
        if (
            formula->move[i - 1] >> 3 == formula->move[i] >> 3
            && formula->move[i - 1] > formula->move[i]
        ) {
            FormulaSwapAdjacent(formula, i++);
        }
    }
}

int FormulaCompare(const Formula* f1, const Formula* f2) {
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


size_t FormulaGenerateIsomorphisms(const Formula* formula, Formula* result) {
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
        result[i].capacity = 32;
        while ((result[i].capacity <<= 1) < length);
        result[i].move = (int*)malloc(result[i].capacity * sizeof(int));
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
            int result_move = (table[move >> 3] << 2) ^ (move & 7);
            move_list[j] = result_move;
            reversed_list[inversed_index] = inverse_move_table[result_move];
            flipped_list[j] = result_move < 16
                ? inverse_move_table[result_move]
                : 40 - result_move;
            reversed_flipped_list[inversed_index] = inverse_move_table[
                flipped_list[j]
            ];
        }
        FormulaNormalize(&result[i]);
        FormulaNormalize(&result[i + 24]);
        FormulaNormalize(&result[i + 48]);
        FormulaNormalize(&result[i + 72]);
    }
    qsort(result, 96, sizeof(Formula), FormulaCompareGeneric);
    Formula* p = result;
    Formula* q = result + 1;
    while (true) {
        while (q != result + 96 && FormulaCompare(p, q) == 0) {
            FormulaDestroy(q++);
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


void CycleReplace(char* c, const char* pattern) {
    const char* position = strchr(pattern, *c);
    if (position) {
        *c = *++position ? *position : *pattern;
    }
}

int FormulaCompareGeneric(const void* p, const void* q) {
    return FormulaCompare((const Formula*)p, (const Formula*)q);
}
