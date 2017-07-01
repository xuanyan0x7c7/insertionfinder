#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


char* Trim(const char* string) {
    const char* begin = string;
    const char* end = string;
    while (*end != '\0') {
        ++end;
    }
    while (begin != end && isspace(*begin)) {
        ++begin;
    }
    while (begin != end && isspace(*(end - 1))) {
        --end;
    }
    char* result = (char*)malloc((end - begin + 1) * sizeof(char));
    strncpy(result, begin, end - begin);
    *(result + (end - begin)) = '\0';
    return result;
}

char* GetLine(FILE* stream) {
    char* string;
    size_t temp = 0;
    if (getline(&string, &temp, stream) == EOF) {
        free(string);
        return NULL;
    }
    char* result = Trim(string);
    free(string);
    return result;
}
