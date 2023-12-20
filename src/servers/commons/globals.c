#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static char *globalString = NULL;  
static bool isInitialized = false;  

void setDatabasePath(char* db) {
    if (!isInitialized) {
        globalString = (char *)malloc(strlen(db) * sizeof(char));  // Allocate memory for a string
        if (globalString == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(globalString, db);
        isInitialized = true;
    }
}

char* getDatabasePath() {
    return globalString;
}

void cleanupDatabasePath() {
    if (isInitialized) {
        free(globalString);
        isInitialized = false;
    }
}