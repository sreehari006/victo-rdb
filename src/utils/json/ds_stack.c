#include "interface/stack.h"
#include "../logs/interface/log.h"

void initializeDSS(DynamicStringStack *stack) {
    stack->top = -1; 
}

int isEmptyDSS(DynamicStringStack *stack) {
    return stack->top == -1;
}

int isFullDSS(DynamicStringStack *stack) {
    return stack->top == STACK_MAX_SIZE - 1;
}

int pushDSS(DynamicStringStack *stack, const char *value) {
    if (isFullDSS(stack)) {
        logWriter(LOG_WARN, "ds_stack pushDSS Stack Overflow");
        return -1;
    }
    stack->top++;
    stack->items[stack->top] = strdup(value);
    if (stack->items[stack->top] == NULL) {
        logWriter(LOG_ERROR, "ds_stack pushDSS Memory allocation failed");
        return -1;
    }
    return 0;
}

char* popDSS(DynamicStringStack *stack) {
    if (isEmptyDSS(stack)) {
        logWriter(LOG_WARN, "ds_stack popDSS Stack is empty");
        return NULL;
    }
    char *value = stack->items[stack->top];
    stack->top--;
    return value;
}

char* peekDSS(DynamicStringStack *stack) {
    if (isEmptyDSS(stack)) {
        logWriter(LOG_WARN, "ds_stack peekDSS Stack is empty");
        return NULL;
    }
    return stack->items[stack->top];
}

void freeDSS(DynamicStringStack *stack) {
    while (!isEmptyDSS(stack)) {
        free(popDSS(stack));
    }
}
