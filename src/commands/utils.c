#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


char* StripNewline(char* string) {
    size_t length = strlen(string);
    while (length) {
        char c = string[length - 1];
        if (c == '\n' || c == '\r') {
            string[--length] = '\0';
        } else {
            break;
        }
    }
    return string;
}

char* GetLine(FILE* stream) {
    char* string;
    size_t temp = 0;
    if (getline(&string, &temp, stream) == EOF) {
        free(string);
        return NULL;
    }
    return StripNewline(string);
}
