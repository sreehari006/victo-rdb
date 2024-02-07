#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} StringBuilder;

void vt__init_string_builder(StringBuilder *builder, size_t initialCapacity);
void vt__append_to_string_uilder(StringBuilder *builder, const char *str);
void vt__free_string_builder(StringBuilder *builder);
bool vt__is_valid_obj_name(char* name);
bool vt__is_valid_integer(const char *str);
