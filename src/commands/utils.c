#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-glib/json-glib.h>
#include "../algorithm/algorithm.h"
#include "../cube/cube.h"
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


bool CubeEqualGeneric(const void* p, const void* q) {
    const int* corner1 = ((const Cube*)p)->corner;
    const int* corner2 = ((const Cube*)q)->corner;
    const int* edge1 = ((const Cube*)p)->edge;
    const int* edge2 = ((const Cube*)q)->edge;
    for (int i = 0; i < 8; ++i) {
        if (corner1[i] != corner2[i]) {
            return false;
        }
    }
    for (int i = 0; i < 12; ++i) {
        if (edge1[i] != edge2[i]) {
            return false;
        }
    }
    return true;
}

size_t CubeHashGeneric(const void* p) {
    const int* corner = ((const Cube*)p)->corner;
    const int* edge = ((const Cube*)p)->edge;
    size_t hash = 0;
    for (int i = 0; i < 8; ++i) {
        hash = hash * 31 + corner[i];
    }
    for (int i = 0; i < 12; ++i) {
        hash = hash * 31 + edge[i];
    }
    return hash;
}

int AlgorithmCompareGeneric(const void* p, const void* q) {
    return AlgorithmCompare(*(const Algorithm**)p, *(const Algorithm**)q);
}

void AlgorithmFreeGeneric(void* p) {
    Algorithm* algorithm = (Algorithm*)p;
    AlgorithmDestroy(algorithm);
    free(algorithm);
}


void PrintJson(JsonNode* json, FILE* stream) {
    JsonGenerator* generator = json_generator_new();
    json_generator_set_root(generator, json);

    gchar* string = json_generator_to_data(generator, NULL);
    g_free(generator);
    fputs(string, stream);
    g_free(string);
}
