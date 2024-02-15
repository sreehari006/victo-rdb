#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "../../commons/constants.h"
#include "../../commons/datastructures.h"
#include "../base/includes/db_config_proto.h"
#include "../file/includes/file_io_proto.h"
#include "../../utils/files/includes/file_utils_proto.h"

Response add_new_collection(const char* db, const char* collection) {
    Response rs;
    char* location = get_collection_full_path(db, collection);
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
    free(location);
    return rs;
}

Response delete_collection(const char* db, const char* collection) {
    Response rs;
    char* location = get_collection_full_path(db, collection);

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

    if(rmdir(location) != 0) {
        rs.errCode = COLLECTION_DELETE_FAILED_ERROR_CODE;
        rs.errMsg = strdup(COLLECTION_DELETE_FAILED_ERROR_MSG);
        free(location);
        return rs;
    } 

    free(location);
    rs.errCode = SUCCESS_CODE;
    rs.errMsg = strdup(SUCESS_MSG);
    return rs;
}

CountRS collection_count(const char* db) {
    CountRS rs;
    char* location = get_collection_base_path(db);

    if(vt__dir_exists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.count = get_directory_count(location);
    } else {
        rs.errCode = DIR_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DIR_NOT_EXIST_ERROR_MSG);
        rs.count = -1;        
    }

    free(location);
    return rs;
}

CollectionListRS collection_list(const char* db) {
    CollectionListRS rs;
    char* location = get_collection_base_path(db);

    if(vt__dir_exists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.collections = list_directory(location);
    } else {
        rs.errCode = DB_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DB_NOT_EXIST_ERROR_MSG);
        rs.collections = NULL;
    }

    free(location);
    return rs;
}
