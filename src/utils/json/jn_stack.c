#include "interface/stack.h"

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
        printf("Stack overflow\n");
        return -1;
    }
    stack->top++;
    stack->items[stack->top] = node;
    if (stack->items[stack->top] == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        // exit(EXIT_FAILURE);
        return -1;
    }
    return 0;
}

JsonNode* popJNS(JsonNodeStack *stack) {
    if (isEmptyJNS(stack)) {
        printf("\n");
        // exit(EXIT_FAILURE);
        return NULL;
    }
    JsonNode *value = stack->items[stack->top];
    stack->top--;
    return value;
}

JsonNode* peekJNS(JsonNodeStack *stack) {
    if (isEmptyJNS(stack)) {
        printf("\nJNS Stack is empty\n");
        // exit(EXIT_FAILURE);
        return NULL;
    }
    return stack->items[stack->top];
}

void freeJNS(JsonNodeStack *stack) {
    while (!isEmptyJNS(stack)) {
        free(popJNS(stack));
    }
}
