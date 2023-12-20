#include <regex.h>
#include <stdbool.h>
#include <limits.h>
#include "interface/string_builder.h"

void initStringBuilder(StringBuilder *builder, size_t initialCapacity) {
    builder->data = (char *)malloc(initialCapacity * sizeof(char));
    builder->data[0] = '\0';
    builder->length = 0;
    builder->capacity = initialCapacity;
}

void appendToStringBuilder(StringBuilder *builder, const char *str) {
    size_t strLength = strlen(str);
    
    while (builder->length + strLength + 1 > builder->capacity) {
        builder->capacity *= 2;
        builder->data = (char *)realloc(builder->data, builder->capacity * sizeof(char));
    }

    strcat(builder->data, str);
    builder->length += strLength;
}

void freeStringBuilder(StringBuilder *builder) {
    free(builder->data);
    builder->data = NULL;
    builder->length = 0;
    builder->capacity = 0;
}

bool isValidObjName(char *str) {
    if (str[0] == '\0') {
        return false;
    }

    if (!((str[0] >= 'a' && str[0] <= 'z') ||
          (str[0] >= 'A' && str[0] <= 'Z') ||
          (str[0] >= '0' && str[0] <= '9'))) {
        return false;
    }

    for (int i = 1; i < strlen(str); i++) {
        if (!((str[i] >= 'a' && str[i] <= 'z') ||
              (str[i] >= 'A' && str[i] <= 'Z') ||
              (str[i] >= '0' && str[i] <= '9') ||
              str[i] == '_' || str[i] == '-')) {
            return false;
        }
    }

    char lastChar = str[strlen(str) - 1];
    if (lastChar == '_' || lastChar == '-') {
        return false;
    }

    return true;
}

bool isValidInteger(const char *str) {
    char *endptr;
    strtol(str, &endptr, 10);
    return (*endptr == '\0' || *endptr == '\n');
}