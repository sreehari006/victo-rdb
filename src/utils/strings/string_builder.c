#include <regex.h>
#include <stdbool.h>
#include <limits.h>
#include "interface/string_builder.h"

void initStringBuilder(StringBuilder *builder, size_t initialCapacity) {
    builder->data = (char *)malloc(initialCapacity * sizeof(char));
    builder->data[0] = '\0';  // Initialize with an empty string
    builder->length = 0;
    builder->capacity = initialCapacity;
}

void appendToStringBuilder(StringBuilder *builder, const char *str) {
    size_t strLength = strlen(str);
    
    // Check if the capacity is sufficient, otherwise, reallocate memory
    while (builder->length + strLength + 1 > builder->capacity) {
        builder->capacity *= 2;
        builder->data = (char *)realloc(builder->data, builder->capacity * sizeof(char));
    }

    // Append the new string
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

int safeAtoi(const char *str) {
    char *endptr;
    long result = strtol(str, &endptr, 10);

    if (*endptr != '\0') {
        // fprintf(stderr, "Error: Invalid input for atoi\n");
        // exit(EXIT_FAILURE);
        return 0;
    }

    if (result > INT_MAX || result < INT_MIN) {
        // fprintf(stderr, "Error: Integer overflow or underflow\n");
        // exit(EXIT_FAILURE);
        return 0;
    }

    return (int) result;
}