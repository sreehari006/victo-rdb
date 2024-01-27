
#include <stdio.h>
#include "interface/user_ops.h"
#include "interface/crypto.h"
#include "../commons/interface/globals.h"
#include "../../utils/logs/interface/log.h"
#include "../../utils/uuid/interface/uuid.h"
#include "../../commons/constants.h"

static char* authFile;
static char* authBasePath;
static pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

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

        // printf(" %s %s %s %d %d %d %d \n", user.name, user.password, user.uuid, user.userAccess, user.dbAccess, user.collectionAccess, user.vectorAccess);
    }

    fclose(file);
}

void initAuthUtil(const char* path) {
    logWriter(LOG_DEBUG, "auth auth started");

    authBasePath = strdup(path);

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
        logWriter(LOG_INFO, passwordHash);

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
    if(authBasePath != NULL) {
        free(authBasePath);
    }

    if(authFile != NULL) {
        free(authFile);
    }

    printf("Auth Util resource cleanup successful.\n");
}

bool findUser(char* userName) {
    bool userExist = false;
    pthread_mutex_lock(&fileMutex);

    FILE* file = fopen(authFile, "rb");
    if (file == NULL) {
        pthread_mutex_unlock(&fileMutex);
        logWriter(LOG_ERROR, "Error find user. Can't open auth data.");
        return false;
    }

    User currentUser;
    while (fread(&currentUser, sizeof(User), 1, file) == 1) {
        if (strcmp(currentUser.name, userName) == 0) {
            userExist = true;
            break;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&fileMutex);

    return userExist;
}

Response addUser(User* user) {
    logWriter(LOG_DEBUG, "auth addUser started");

    Response rs;
    pthread_mutex_lock(&fileMutex);

    FILE* file = fopen(authFile, "ab");
    if (file == NULL) {
        pthread_mutex_unlock(&fileMutex);
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        logWriter(LOG_ERROR, "Error adding new user. Can't open auth data.");
        return rs;
    }

    bool userExist = findUser(strdup(user->name));
    if(userExist) {
        fclose(file);
        pthread_mutex_unlock(&fileMutex);
        rs.errCode = USER_ALREADY_EXIST_CODE;
        rs.errMsg = strdup(USER_ALREADY_EXIST_MSG);
        logWriter(LOG_ERROR, "Error adding new user. User already exist.");
        return rs;        
    }

    if(fwrite(user, sizeof(User), 1, file) != 1) {
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        logWriter(LOG_ERROR, "Error adding new user. File open error.");
    } else {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
    }
    
    fclose(file);
    pthread_mutex_unlock(&fileMutex);


    logWriter(LOG_DEBUG, "auth addUser completed");

    return rs;
}


Response updateUser(char* userName, User* user) {
    logWriter(LOG_DEBUG, "auth updateUser started");
    
    Response rs;
    pthread_mutex_lock(&fileMutex);

    FILE* file = fopen(authFile, "r+b");
    if (file == NULL) {
        pthread_mutex_unlock(&fileMutex);
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        logWriter(LOG_ERROR, "Error updating user. Can't open auth data.");
        return rs;
    }

    User currentUser;
    while (fread(&currentUser, sizeof(User), 1, file) == 1) {
        if (strcmp(currentUser.name, userName) == 0) {
            fseek(file, -sizeof(User), SEEK_CUR);
            if(fwrite(user, sizeof(User), 1, file) != 1) {
                rs.errCode = FAILED_CODE;
                rs.errMsg = strdup(FAILED_MSG);
                logWriter(LOG_ERROR, "Error updating user. File open error.");
            } else {
                rs.errCode = SUCCESS_CODE;
                rs.errMsg = strdup(SUCESS_MSG);
            }

            break;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&fileMutex);
    
    logWriter(LOG_DEBUG, "auth updateUser completed");

    return rs;
}

User* authenticate(char* userName, char* password) {

    if(userName == NULL) {
        logWriter(LOG_ERROR, "Error authenticate user. Invalid userName.");
        return NULL;
    }

    if(password == NULL) {
        logWriter(LOG_ERROR, "Error authenticate user. Invalid password.");
        return NULL;
    }

    pthread_mutex_lock(&fileMutex);

    FILE* file = fopen(authFile, "rb");
    if (file == NULL) {
        logWriter(LOG_ERROR, "Error authenticate user. Can't open auth data.");
        pthread_mutex_unlock(&fileMutex);
        return NULL;
    }

    User* currentUser = (User*)malloc(sizeof(User));
    
    while (fread(currentUser, sizeof(User), 1, file) == 1) {
        if (strcmp(currentUser->name, userName) == 0 && strcmp(currentUser->password, password) == 0) {
            fclose(file);
            pthread_mutex_unlock(&fileMutex);
            return currentUser;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&fileMutex);

    return NULL;
}

Response deleteUser(char* userName) {
    logWriter(LOG_DEBUG, "delete deleteUser started");
    
    Response rs;
    pthread_mutex_lock(&fileMutex);

    FILE *file = fopen(authFile, "rb+");
    if (file == NULL) {
        pthread_mutex_unlock(&fileMutex);
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        logWriter(LOG_ERROR, "Error deleting user. Can't open auth data.");
        return rs;
    }

    int len = strlen(authBasePath) + strlen("/tempfile.tmp") + 1;
    char* tempFilePath = (char *)malloc(len);
    strcpy(tempFilePath, authBasePath);
    strcat(tempFilePath, "/tempfile.tmp");
    
    FILE *tempFile = fopen(tempFilePath, "wb");
    if (tempFile == NULL) {
        fclose(file);
        pthread_mutex_unlock(&fileMutex);
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        logWriter(LOG_ERROR, "Error deleting user. Can't open temporary file.");
        return rs;
    }

    User currentUser;

    while (fread(&currentUser, sizeof(User), 1, file) == 1) {
        if (strcmp(currentUser.name, userName) == 0) {
            continue;
        }

        if(fwrite(&currentUser, sizeof(User), 1, tempFile) != 1) {
            fclose(file);
            fclose(tempFile);
            remove(tempFilePath);

            pthread_mutex_unlock(&fileMutex);
            
            rs.errCode = FAILED_CODE;
            rs.errMsg = strdup(FAILED_MSG);

            logWriter(LOG_ERROR, "Error deleting user. Error writing to a temporary file.");
            return rs;
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(authFile);
    rename(tempFilePath, authFile);

    pthread_mutex_unlock(&fileMutex);

    logWriter(LOG_DEBUG, "delete deleteUser completed");

    rs.errCode = SUCCESS_CODE;
    rs.errMsg = strdup(SUCESS_MSG);    
    return rs;
}

void freeUser(User* user) {
    if(user != NULL) {
        free(user->name);
        free(user->password);
        free(user->uuid);
        free(user);
    }
}