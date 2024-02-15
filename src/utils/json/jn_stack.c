#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./includes/stack_proto.h"
#include "../logs/includes/log_proto.h"

void vt__initialize_JNS(JsonNodeStack *stack) {
    stack->top = -1; 
}

int vt__is_empty_JNS(JsonNodeStack *stack) {
    return stack->top == -1;
}

int vt__is_full_JNS(JsonNodeStack *stack) {
    return stack->top == STACK_MAX_SIZE - 1;
}

int vt__push_JNS(JsonNodeStack *stack, JsonNode* node) {
        if (vt__is_full_JNS(stack)) {
        vt__log_writer(LOG_WARN, "jn_stack pushJNS Stack overflow");
        return -1;
    }
    stack->top++;
    stack->items[stack->top] = node;
    if (stack->items[stack->top] == NULL) {
        vt__log_writer(LOG_ERROR, "jn_stakc pushJNS Memory allocation failed");
        return -1;
    }
    
    return 0;
}

JsonNode* vt__pop_JNS(JsonNodeStack *stack) {
    if (vt__is_empty_JNS(stack)) {
        vt__log_writer(LOG_WARN, "jn_stack popJNS Stack is empty");
        return NULL;
    }
    JsonNode *value = stack->items[stack->top];
    stack->top--;

    return value;
}

JsonNode* vt__peek_JNS(JsonNodeStack *stack) {
    if (vt__is_empty_JNS(stack)) {
        vt__log_writer(LOG_WARN, "jn_stack peekJNS Stack is empty");
        return NULL;
    }
    return stack->items[stack->top];
}

void vt__free_JNS(JsonNodeStack *stack) {
    while (!vt__is_empty_JNS(stack)) {
        free(vt__pop_JNS(stack));
    }
}
