
#include <stdio.h>
#include "interface/auth.h"
#include "../commons/interface/globals.h"
#include "../../utils/logs/interface/log.h"

static char* authFile;

void initAuthUtil(const char* path) {
    logWriter(LOG_DEBUG, "auth auth started");

    int len = strlen(path) + strlen("/users.bin") + 1;
    authFile = (char *)malloc(len);

    strcpy(authFile, path);
    strcat(authFile, "/users.bin");
    free((void *) path);

    logWriter(LOG_INFO, "Auth Location: ");
    logWriter(LOG_INFO, authFile);

    if (authFile == NULL) {
        logWriter(LOG_ERROR, "Auth Module Initialization failed");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(authFile, "r+");
    if(file == NULL) {
        logWriter(LOG_WARN, "Auth data does not exist. Creating new....");
        file = fopen(authFile, "w+");
        if (file == NULL) {
            logWriter(LOG_ERROR, "Error initializing Auth data");
            exit(EXIT_FAILURE);
        }
        logWriter(LOG_INFO, "New Auth data created");
    } else {
        logWriter(LOG_INFO, "Auth data already exist");
    }

    fclose(file);

    logWriter(LOG_DEBUG, "auth auth completed");
}

void freeAuthUtil() {
    if(authFile != NULL) {
        free(authFile);
    }
    printf("Auth Util resource cleanup successful.\n");
}
