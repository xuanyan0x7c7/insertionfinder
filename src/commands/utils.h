#pragma once
#include <stdio.h>
#include <config.h>
#if HAVE_JSON
#include <json-glib/json-glib.h>
#endif

char* trim(const char* string);
char* get_line(FILE* stream);

bool cube_equal_generic(const void* p, const void* q);
size_t cube_hash_generic(const void* p);
int algorithm_compare_generic(const void* p, const void* q);
void algorithm_free_generic(void* p);

#if HAVE_JSON
    void print_json(JsonNode* json, FILE* stream);
#endif
