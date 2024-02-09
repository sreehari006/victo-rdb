
#include <stdio.h>
#include "./includes/user_ops_proto.h"
#include "./includes//crypto_proto.h"
#include "../commons/includes/globals_proto.h"
#include "../../utils/logs/includes/log_proto.h"
#include "../../utils/uuid/includes/uuid_proto.h"
#include "../../commons/constants.h"

static char* auth_file;
static char* auth_base_path;
static pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

void load_auth_data() {
    FILE *file = fopen(auth_file, "rb");

    if (file == NULL) {
        vt__log_writer(LOG_ERROR, "Auth data does not exist");
        exit(EXIT_FAILURE);
    }

    while (1) {
        User user;

        size_t bytesRead = fread(&user, sizeof(User), 1, file);

        if (bytesRead != 1) {
            if (feof(file)) {
                break;  
            } else {
                vt__log_writer(LOG_ERROR, "Error reading Auth data");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }

    }

    fclose(file);
}

void init_auth_util(const char* path) {
    vt__log_writer(LOG_DEBUG, "auth auth started");

    auth_base_path = strdup(path);

    int len = strlen(path) + strlen("/users.bin") + 1;
    auth_file = (char *)malloc(len);

    strcpy(auth_file, path);
    strcat(auth_file, "/users.bin");
    
    vt__log_writer(LOG_INFO, "Auth Location: ");
    vt__log_writer(LOG_INFO, auth_file);

    if (auth_file == NULL) {
        vt__log_writer(LOG_ERROR, "Auth Module Initialization failed");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(auth_file, "r+");
    if(file == NULL) {
        vt__log_writer(LOG_WARN, "Auth data does not exist. Creating new....");
        file = fopen(auth_file, "w+");
        if (file == NULL) {
            vt__log_writer(LOG_ERROR, "Error initializing Auth data");
            exit(EXIT_FAILURE);
        }
        vt__log_writer(LOG_INFO, "New Auth data created");

        User user;
        char* password = vt__get_uuid();
        char* passwordHash = vt__sha256(password);

        if(passwordHash == NULL) {
            vt__log_writer(LOG_ERROR, "Error creating Admin User");
            exit(EXIT_FAILURE);            
        }

        char* uuid = vt__get_uuid();
    
        strcpy(user.name, "admin");
        strcpy(user.password, passwordHash);
        strcpy(user.uuid, uuid);
        user.user_access[USER_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;
        user.user_access[DB_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;
        user.user_access[COLLECTION_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;
        user.user_access[VECTOR_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;
        user.user_access[SUBSCRIPTION_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;

        vt__log_writer(LOG_INFO, "Admin Password: ");
        vt__log_writer(LOG_INFO, password);
        
        free(password);
        free(passwordHash);
        free(uuid);

        if (fwrite(&user, sizeof(User), 1, file) != 1) {
            vt__log_writer(LOG_ERROR, "Error writing Admin User to Auth data");
            exit(EXIT_FAILURE); 
        }

    } else {
        vt__log_writer(LOG_INFO, "Auth data already exist");
    }

    fclose(file);

    load_auth_data();
    vt__log_writer(LOG_DEBUG, "auth auth completed");
}

void free_auth_util() {
    vt__log_writer(LOG_DEBUG, "auth freeAuthUtil started");
    
    if(auth_base_path != NULL) {
        free(auth_base_path);
    }

    if(auth_file != NULL) {
        free(auth_file);
    }

    vt__log_writer(LOG_DEBUG, "auth freeAuthUtil completed");
}

bool find_user(char* user_name) {
    bool userExist = false;
    pthread_mutex_lock(&file_mutex);

    FILE* file = fopen(auth_file, "rb");
    if (file == NULL) {
        pthread_mutex_unlock(&file_mutex);
        vt__log_writer(LOG_ERROR, "Error find user. Can't open auth data.");
        return false;
    }

    User currentUser;
    while (fread(&currentUser, sizeof(User), 1, file) == 1) {
        if (strcmp(currentUser.name, user_name) == 0) {
            userExist = true;
            break;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&file_mutex);

    return userExist;
}

Response add_user(User* user) {
    vt__log_writer(LOG_DEBUG, "auth addUser started");

    Response rs;
    bool userExist = find_user(strdup(user->name));
    if(userExist) {
        rs.errCode = USER_ALREADY_EXIST_CODE;
        rs.errMsg = strdup(USER_ALREADY_EXIST_MSG);
        vt__log_writer(LOG_ERROR, "Error adding new user. User already exist.");
        return rs;        
    }

    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(auth_file, "ab");
    if (file == NULL) {
        pthread_mutex_unlock(&file_mutex);
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        vt__log_writer(LOG_ERROR, "Error adding new user. Can't open auth data.");
        return rs;
    }

    if(fwrite(user, sizeof(User), 1, file) != 1) {
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        vt__log_writer(LOG_ERROR, "Error adding new user. File open error.");
    } else {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
    }
    
    fclose(file);
    pthread_mutex_unlock(&file_mutex); 


    vt__log_writer(LOG_DEBUG, "auth addUser completed"); 

    return rs;
}


Response update_user(char* user_id, User* user) {
    vt__log_writer(LOG_DEBUG, "auth updateUser started");
    
    Response rs;
    pthread_mutex_lock(&file_mutex);

    FILE* file = fopen(auth_file, "r+b");
    if (file == NULL) {
        pthread_mutex_unlock(&file_mutex);
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        vt__log_writer(LOG_ERROR, "Error updating user. Can't open auth data.");
        return rs;
    }

    User currentUser;
    while (fread(&currentUser, sizeof(User), 1, file) == 1) {
        if (strcmp(currentUser.uuid, user_id) == 0) {
            fseek(file, -sizeof(User), SEEK_CUR);
            if(fwrite(user, sizeof(User), 1, file) != 1) {
                rs.errCode = FAILED_CODE;
                rs.errMsg = strdup(FAILED_MSG);
                vt__log_writer(LOG_ERROR, "Error updating user. File open error.");
            } else {
                rs.errCode = SUCCESS_CODE;
                rs.errMsg = strdup(SUCESS_MSG);
            }

            break;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&file_mutex);
    
    vt__log_writer(LOG_DEBUG, "auth updateUser completed");

    return rs;
}

User* authenticate(char* user_name, char* password) {

    if(user_name == NULL) {
        vt__log_writer(LOG_ERROR, "Error authenticate user. Invalid userName.");
        return NULL;
    }

    if(password == NULL) {
        vt__log_writer(LOG_ERROR, "Error authenticate user. Invalid password.");
        return NULL;
    }

    pthread_mutex_lock(&file_mutex);

    FILE* file = fopen(auth_file, "rb");
    if (file == NULL) {
        vt__log_writer(LOG_ERROR, "Error authenticate user. Can't open auth data.");
        pthread_mutex_unlock(&file_mutex);
        return NULL;
    }

    User* currentUser = (User*)malloc(sizeof(User));
    
    while (fread(currentUser, sizeof(User), 1, file) == 1) {
        if (strcmp(currentUser->name, user_name) == 0 && strcmp(currentUser->password, password) == 0) {
            fclose(file);
            pthread_mutex_unlock(&file_mutex);
            return currentUser;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&file_mutex);

    return NULL;
}

Response delete_user(char* user_name) {
    vt__log_writer(LOG_DEBUG, "delete deleteUser started");
    
    Response rs;
    pthread_mutex_lock(&file_mutex);

    FILE *file = fopen(auth_file, "rb+");
    if (file == NULL) {
        pthread_mutex_unlock(&file_mutex);
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        vt__log_writer(LOG_ERROR, "Error deleting user. Can't open auth data.");
        return rs;
    }

    int len = strlen(auth_base_path) + strlen("/tempfile.tmp") + 1;
    char* tempFilePath = (char *)malloc(len);
    strcpy(tempFilePath, auth_base_path);
    strcat(tempFilePath, "/tempfile.tmp");
    
    FILE *tempFile = fopen(tempFilePath, "wb");
    if (tempFile == NULL) {
        fclose(file);
        pthread_mutex_unlock(&file_mutex);
        rs.errCode = FAILED_CODE;
        rs.errMsg = strdup(FAILED_MSG);
        vt__log_writer(LOG_ERROR, "Error deleting user. Can't open temporary file.");
        return rs;
    }

    User currentUser;

    while (fread(&currentUser, sizeof(User), 1, file) == 1) {
        if (strcmp(currentUser.name, user_name) == 0) {
            continue;
        }

        if(fwrite(&currentUser, sizeof(User), 1, tempFile) != 1) {
            fclose(file);
            fclose(tempFile);
            remove(tempFilePath);

            pthread_mutex_unlock(&file_mutex);
            
            rs.errCode = FAILED_CODE;
            rs.errMsg = strdup(FAILED_MSG);

            vt__log_writer(LOG_ERROR, "Error deleting user. Error writing to a temporary file.");
            return rs;
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(auth_file);
    rename(tempFilePath, auth_file);

    pthread_mutex_unlock(&file_mutex);

    vt__log_writer(LOG_DEBUG, "delete deleteUser completed");

    rs.errCode = SUCCESS_CODE;
    rs.errMsg = strdup(SUCESS_MSG);    
    return rs;
}
