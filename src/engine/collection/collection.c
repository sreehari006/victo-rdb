#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <shlwapi.h>
    #pragma comment(lib, "Shlwapi.lib")
#else
    #include <dirent.h>
    #include <limits.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <errno.h>
#endif
#include "interface/collection.h"
#include "../../commons/constants.h"
#include "../../ds/datastructures.h"
#include "../file/interface/file_io.h"

__attribute__((visibility("hidden"))) 
Response newCollection(const char* location) {
    Response rs;
    #if defined(_WIN32) || defined(_WIN64)
        if(_mkdir(location) == 0) {
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = SUCESS_MSG;
        } else {
            rs.errCode = SCHEMA_FAILED_ERROR_CODE;
            rs.errMsg = SCHEMA_FAILED_ERROR_MSG;
        }
    #else
        if(mkdir(location, 0777) == 0) {
            char vectors[strlen(location) + strlen(VECTORS) + 1];
            strcpy(vectors, location);
            strcat(vectors, "/");
            strcat(vectors, VECTORS);
            mkdir(vectors, 0777);

            char subscriptions[strlen(location) + strlen(SUBSCRIPTIONS) + 1];
            strcpy(subscriptions, location);
            strcat(subscriptions, "/");
            strcat(subscriptions, SUBSCRIPTIONS);
            mkdir(subscriptions, 0777);

            rs.errCode = SUCCESS_CODE;
            rs.errMsg = strdup(SUCESS_MSG);
        } else {
            if(errno == EEXIST) {
                rs.errCode = COLLECTION_EXIST_ERROR_CODE;
                rs.errMsg = strdup(COLLECTION_EXIST_ERROR_MSG);
            } else {
                rs.errCode = COLLECTION_CREATE_FAILED_ERROR_CODE;
                rs.errMsg = strdup(COLLECTION_CREATE_FAILED_ERROR_MSG);
            }
        }
    #endif

    return rs;
}

__attribute__((visibility("hidden"))) 
Response deleteCollection(const char* location) {
    Response rs;
    char vectors[strlen(location) + strlen(VECTORS) + 1];
    strcpy(vectors, location);
    strcat(vectors, "/");
    strcat(vectors, VECTORS);
    int errCode = delete_victo_collection(vectors);
    rmdir(vectors);

    char subscriptions[strlen(location) + strlen(SUBSCRIPTIONS) + 1];
    strcpy(subscriptions, location);
    strcat(subscriptions, "/");
    strcat(subscriptions, SUBSCRIPTIONS);
    rmdir(subscriptions);

    #if defined(_WIN32) || defined(_WIN64)
        if(!RemoveDirectory(location)) {
            rs.errCode = COLLECTION_DELETE_FAILED_ERROR_CODE;
            rs.errMsg = COLLECTION_DELETE_FAILED_ERROR_MSG;
            return rs;            
        }
    #else
        if(rmdir(location) != 0) {
            rs.errCode = COLLECTION_DELETE_FAILED_ERROR_CODE;
            rs.errMsg = strdup(COLLECTION_DELETE_FAILED_ERROR_MSG);
            return rs;
        } 
    #endif

    rs.errCode = SUCCESS_CODE;
    rs.errMsg = strdup(SUCESS_MSG);
    return rs;
}

__attribute__((visibility("hidden"))) 
CountRS collectionCount(const char* location) {
    CountRS rs;
    if(dirExists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.count = get_directory_count(location);
    } else {
        rs.errCode = DIR_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DIR_NOT_EXIST_ERROR_MSG);
        rs.count = -1;        
    }
    return rs;
}

__attribute__((visibility("hidden"))) 
CollectionListRS collectionList(const char* location) {
    CollectionListRS rs;
    if(dirExists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.collections = list_directory(location);
    } else {
        rs.errCode = DB_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DB_NOT_EXIST_ERROR_MSG);
    }
    return rs;
}
