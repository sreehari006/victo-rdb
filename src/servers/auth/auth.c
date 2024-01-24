
#include <stdio.h>
#include "interface/auth.h"
#include "interface/crypto.h"
#include "../commons/interface/globals.h"
#include "../../utils/logs/interface/log.h"
#include "../../utils/uuid/interface/uuid.h"

static char* authFile;

void loadAuthData() {
    FILE *file = fopen(authFile, "rb");

    if (file == NULL) {
        logWriter(LOG_ERROR, "Auth data does not exist");
        exit(EXIT_FAILURE);
    }

    while (1) {
        User user;

        size_t bytesRead = fread(&user, sizeof(User), 1, file);

        if (bytesRead != 1) {
            if (feof(file)) {
                break;  
            } else {
                logWriter(LOG_ERROR, "Error reading Auth data");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }

        printf(" %s %s %s %d %d %d %d \n", user.name, user.password, user.uuid, user.userAccess, user.dbAccess, user.collectionAccess, user.vectorAccess);
    }

    fclose(file);
}

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

        User user;
        char* password = getUUID();
        char* passwordHash = sha256(password);

        if(passwordHash == NULL) {
            logWriter(LOG_ERROR, "Error creating Admin User");
            exit(EXIT_FAILURE);            
        }

        char* uuid = getUUID();
    
        strcpy(user.name, "admin");
        strcpy(user.password, passwordHash);
        strcpy(user.uuid, uuid);
        user.userAccess = 32767;
        user.dbAccess = 32767;
        user.collectionAccess = 32767;
        user.vectorAccess = 32767;

        logWriter(LOG_INFO, "Admin Password: ");
        logWriter(LOG_INFO, password);

        free(password);
        free(passwordHash);
        free(uuid);

        if (fwrite(&user, sizeof(User), 1, file) != 1) {
            logWriter(LOG_ERROR, "Error writing Admin User to Auth data");
            exit(EXIT_FAILURE); 
        }

    } else {
        logWriter(LOG_INFO, "Auth data already exist");
    }

    fclose(file);

    loadAuthData();
    logWriter(LOG_DEBUG, "auth auth completed");
}

void freeAuthUtil() {
    if(authFile != NULL) {
        free(authFile);
    }
    printf("Auth Util resource cleanup successful.\n");
}
