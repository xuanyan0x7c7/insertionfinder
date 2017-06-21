#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "data-structure/linked-list.h"
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

static const int inv_move[] = {
    0, 3, 2, 1,
    4, 7, 6, 5,
    8, 11, 10, 9,
    12, 15, 14, 13,
    16, 19, 18, 17,
    20, 23, 22, 21
};


static inline char* ListGetItem(const LinkedListNode* node);

static void CycleReplace(char* c, const char* pattern);


Formula* FormulaConstruct(Formula* formula, const char* string) {
    static regex_t moves_regex;
    static bool regex_inited = false;
    if (!regex_inited) {
        int status = regcomp(
            &moves_regex,
            "([UDRLFBxyz](2'?|'|w2'?|w|)|\\[[udrlfb](2'?|'|)\\])",
            REG_EXTENDED
        );
        if (status) {
            char message[100];
            regerror(status, &moves_regex, message, 99);
            fputs(message, stderr);
            return NULL;
        }
        regex_inited = true;
    }

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

    LinkedList procedure;
    LinkedListConstruct(&procedure, free);

    const char* buffer = string;
    regmatch_t pmatch[4];
    while (buffer && regexec(&moves_regex, buffer, 3, pmatch, 0) == 0) {
        regmatch_t* match = pmatch;
        size_t length = match->rm_eo - match->rm_so;
        char* move_string = (char*)malloc((length + 1) * sizeof(char));
        strncpy(move_string, buffer + match->rm_so, length);
        move_string[length] = '\0';
        char* position = strstr(move_string, "2'");
        if (position) {
            while (*position) {
                *position = *(position + 1);
                ++position;
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
                if (strcmp(ListGetItem(pointer), (*matches)[j]) == 0) {
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
                            CycleReplace(ListGetItem(node), pattern);
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

    if (!formula) {
        formula = (Formula*)malloc(sizeof(Formula));
    }
    formula->length = 0;
    formula->capacity = 16;
    formula->move = (int*)malloc(formula->capacity * sizeof(int));
    for (
        const LinkedListNode* node = procedure.head;
        (node = node->next) != procedure.tail;
    ) {
        for (size_t i = 0; i < ArrayLength(twist_str); ++i) {
            if (strcmp(ListGetItem(node), twist_str[i]) == 0) {
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


char* FormulaToString(const Formula* formula, char* string) {
    if (formula->length == 0) {
        if (string) {
            return strcpy(string, "");
        } else {
            return strdup("");
        }
    }

    if (!string) {
        size_t string_length = formula->length - 1;
        for (size_t i = 0; i < formula->length; ++i) {
            string_length += strlen(twist_str[formula->move[i]]);
        }
        string = (char*)malloc((string_length + 1) * sizeof(char));
    }

    strcpy(string, twist_str[formula->move[0]]);
    size_t offset = strlen(twist_str[formula->move[0]]);
    for (size_t i = 1; i < formula->length; ++i) {
        strcpy(string + offset++, " ");
        const char* s = twist_str[formula->move[i]];
        strcpy(string + offset, s);
        offset += strlen(s);
    }
    return string;
}


size_t FormulaCancelMoves(Formula* formula) {
    int x = 0;
    int y = -1;
    int length = formula->length;
    int index = length + 1;
    int* move = formula->move;

    while (x < length) {
        if (y < 0 || move[x] >> 3 != move[y] >> 3) {
            if (++y != x++) {
                move[y] = move[x - 1];
            }
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
                    move[y - 1] = (move[y - 1] & ~3) + orientation;
                }
            } else {
                move[++y] = move[x++];
            }
        } else {
            int orientation = (move[y] + move[x++]) & 3;
            if (orientation == 0) {
                --y;
            } else {
                move[y] = (move[y] & ~3) + orientation;
            }
            if (index > y + 1) {
                index = y + 1;
            }
        }
    }

    formula->length = y + 1;
    return index;
}


char* ListGetItem(const LinkedListNode* node) {
    return (char*)node->data;
}


void CycleReplace(char* c, const char* pattern) {
    const char* position = strchr(pattern, *c);
    if (position) {
        *c = *++position ? *position : *pattern;
    }
}
