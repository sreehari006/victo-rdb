#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./includes/stack_proto.h"
#include "../logs/includes/log_proto.h"

void vt__initialize_DSS(DynamicStringStack *stack) {
    stack->top = -1; 
}

int vt__is_empty_DSS(DynamicStringStack *stack) {
    return stack->top == -1;
}

int vt__is_full_DSS(DynamicStringStack *stack) {
    return stack->top == STACK_MAX_SIZE - 1;
}

int vt__push_DSS(DynamicStringStack *stack, const char *value) {
    if (vt__is_full_DSS(stack)) {
        vt__log_writer(LOG_WARN, "ds_stack pushDSS Stack Overflow");
        return -1;
    }
    stack->top++;
    stack->items[stack->top] = strdup(value);
    if (stack->items[stack->top] == NULL) {
        vt__log_writer(LOG_ERROR, "ds_stack pushDSS Memory allocation failed");
        return -1;
    }
    return 0;
}

char* vt__pop_DSS(DynamicStringStack *stack) {
    if (vt__is_empty_DSS(stack)) {
        vt__log_writer(LOG_WARN, "ds_stack popDSS Stack is empty");
        return NULL;
    }
    char *value = stack->items[stack->top];
    stack->top--;
    return value;
}

char* vt__peek_DSS(DynamicStringStack *stack) {
    if (vt__is_empty_DSS(stack)) {
        vt__log_writer(LOG_WARN, "ds_stack peekDSS Stack is empty");
        return NULL;
    }
    return stack->items[stack->top];
}

void vt__free_DSS(DynamicStringStack *stack) {
    while (!vt__is_empty_DSS(stack)) {
        free(vt__pop_DSS(stack));
    }
}
