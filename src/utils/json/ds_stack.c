#include "interface/stack.h"

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
        printf("Stack overflow\n");
        return -1;
    }
    stack->top++;
    stack->items[stack->top] = strdup(value);
    if (stack->items[stack->top] == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        // exit(EXIT_FAILURE);
        return -1;
    }
    return 0;
}

char* popDSS(DynamicStringStack *stack) {
    if (isEmptyDSS(stack)) {
        printf("\nDynamic String Stack Stack underflow\n");
        // exit(EXIT_FAILURE);
        return NULL;
    }
    char *value = stack->items[stack->top];
    stack->top--;
    return value;
}

char* peekDSS(DynamicStringStack *stack) {
    if (isEmptyDSS(stack)) {
        printf("Stack is empty\n");
        // exit(EXIT_FAILURE);
        return NULL;
    }
    return stack->items[stack->top];
}

void freeDSS(DynamicStringStack *stack) {
    while (!isEmptyDSS(stack)) {
        free(popDSS(stack));
    }
}
