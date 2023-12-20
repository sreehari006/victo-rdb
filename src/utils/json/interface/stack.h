#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./json.h"

#define STACK_MAX_SIZE 16384

typedef struct {
    char* items[STACK_MAX_SIZE];
    int top;
} DynamicStringStack;


void initializeDSS(DynamicStringStack *stack);
int isEmptyDSS(DynamicStringStack *stack);
int isFullDSS(DynamicStringStack *stack);
int pushDSS(DynamicStringStack *stack, const char *value);
char* popDSS(DynamicStringStack *stack);
char* peekDSS(DynamicStringStack *stack);
void freeDSS(DynamicStringStack *stack);


typedef struct {
    JsonNode* items[STACK_MAX_SIZE];
    int top;
} JsonNodeStack;

void initializeJNS(JsonNodeStack *stack);
int isEmptyJNS(JsonNodeStack *stack);
int isFullJNS(JsonNodeStack *stack);
int pushJNS(JsonNodeStack *stack, JsonNode* node);
JsonNode* popJNS(JsonNodeStack *stack);
JsonNode* peekJNS(JsonNodeStack *stack);
void freeJNS(JsonNodeStack *stack);