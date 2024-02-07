#include "./json_ds_proto.h"

#define STACK_MAX_SIZE 2048

typedef struct {
    char* items[STACK_MAX_SIZE];
    int top;
} DynamicStringStack;

typedef struct {
    JsonNode* items[STACK_MAX_SIZE];
    int top;
} JsonNodeStack;


void vt__initialize_DSS(DynamicStringStack *stack);
int vt__is_empty_DSS(DynamicStringStack *stack);
int vt__is_full_DSS(DynamicStringStack *stack);
int vt__push_DSS(DynamicStringStack *stack, const char *value);
char* vt__pop_DSS(DynamicStringStack *stack);
char* vt__peek_DSS(DynamicStringStack *stack);
void vt__free_DSS(DynamicStringStack *stack);

void vt__initialize_JNS(JsonNodeStack *stack);
int vt__is_empty_JNS(JsonNodeStack *stack);
int vt__is_full_JNS(JsonNodeStack *stack);
int vt__push_JNS(JsonNodeStack *stack, JsonNode* node);
JsonNode* vt__pop_JNS(JsonNodeStack *stack);
JsonNode* vt__peek_JNS(JsonNodeStack *stack);
void vt__free_JNS(JsonNodeStack *stack);