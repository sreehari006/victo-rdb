#include "interface/stack.h"
#include "../logs/interface/log.h"

void initializeJNS(JsonNodeStack *stack) {
    stack->top = -1; 
}

int isEmptyJNS(JsonNodeStack *stack) {
    return stack->top == -1;
}

int isFullJNS(JsonNodeStack *stack) {
    return stack->top == STACK_MAX_SIZE - 1;
}

int pushJNS(JsonNodeStack *stack, JsonNode* node) {
        if (isFullJNS(stack)) {
        logWriter(LOG_WARN, "jn_stack pushJNS Stack overflow");
        return -1;
    }
    stack->top++;
    stack->items[stack->top] = node;
    if (stack->items[stack->top] == NULL) {
        logWriter(LOG_ERROR, "jn_stakc pushJNS Memory allocation failed");
        return -1;
    }
    return 0;
}

JsonNode* popJNS(JsonNodeStack *stack) {
    if (isEmptyJNS(stack)) {
        logWriter(LOG_WARN, "jn_stack popJNS Stack is empty");
        return NULL;
    }
    JsonNode *value = stack->items[stack->top];
    stack->top--;
    return value;
}

JsonNode* peekJNS(JsonNodeStack *stack) {
    if (isEmptyJNS(stack)) {
        logWriter(LOG_WARN, "jn_stack peekJNS Stack is empty");
        return NULL;
    }
    return stack->items[stack->top];
}

void freeJNS(JsonNodeStack *stack) {
    while (!isEmptyJNS(stack)) {
        free(popJNS(stack));
    }
}
