#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} StringBuilder;

void initStringBuilder(StringBuilder *builder, size_t initialCapacity);
void appendToStringBuilder(StringBuilder *builder, const char *str);
void freeStringBuilder(StringBuilder *builder);
bool isValidObjName(char* name);