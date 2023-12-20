#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct JsonNode {
    struct JsonNode* parent;
    struct JsonNode* children[8192];
    char* type;
    char* key;
    char* value;
    int childIndex;
} JsonNode;

typedef enum {
    TOKEN_NONE,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET
} TokenType;

JsonNode* loadJson(char* jsonStr);
JsonNode* searchJson(JsonNode* node, char* key);
void printJson(JsonNode* node);
void freeJson(JsonNode* node);