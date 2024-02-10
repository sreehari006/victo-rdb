#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include "../../commons/constants.h"
#include "../../utils/files/includes/file_utils_proto.h"
#include "../../utils/strings/includes/string_builder_proto.h"

static char* victo_base_path;
static char* db_base_path;
static char* logs_base_path;
static char* auth_base_path;


void init_victo_config(const char* location) {
    if(!vt__dir_exists(location)) {
        perror("The path provided to start victo server does not exist.");
        exit(EXIT_FAILURE);
    }   

    victo_base_path = strdup(location);

    db_base_path = (char*)malloc(strlen(location) + strlen(DB) + 2);
    if(db_base_path == NULL) {
        perror("The base path creation for db failed.");
        exit(EXIT_FAILURE);
    }
    strcpy(db_base_path, location);
    strcat(db_base_path, VT_FILE_SEPERATOR);
    strcat(db_base_path, DB);

    logs_base_path = (char*)malloc(strlen(location) + strlen(LOGS) + 2);
    if(logs_base_path == NULL) {
        perror("The base path creation for logs failed.");
        exit(EXIT_FAILURE);
    }
    strcpy(logs_base_path, location);
    strcat(logs_base_path, VT_FILE_SEPERATOR);
    strcat(logs_base_path, LOGS);

    auth_base_path = (char*)malloc(strlen(location) + strlen(AUTH) + 2);
    if(auth_base_path == NULL) {
        perror("The base path creation for auth data failed.");
        exit(EXIT_FAILURE);
    }
    strcpy(auth_base_path, location);
    strcat(auth_base_path, VT_FILE_SEPERATOR);
    strcat(auth_base_path, AUTH);

    if(!vt__dir_exists(db_base_path)) {
        if(mkdir(db_base_path, 0777) != 0) {
            perror("The base path creation for db failed.");
            exit(EXIT_FAILURE);
        }
    }

    if(!vt__dir_exists(logs_base_path)) {
        if(mkdir(logs_base_path, 0777) != 0) {
            perror("The base path creation for logs failed.");
            exit(EXIT_FAILURE);
        }
    }

    if(!vt__dir_exists(auth_base_path)) {
        if(mkdir(auth_base_path, 0777) != 0) {
            perror("The base path creation for auth data failed.");
            exit(EXIT_FAILURE);
        }
    }
}

char* get_victo_base_path() {
    return strdup(victo_base_path);
}

char* get_db_base_path() {
    return strdup(db_base_path);
}

char* get_logs_base_path() {
    return strdup(logs_base_path);
}

char* get_auth_base_path() {
    return strdup(auth_base_path);
}

void free_victo_config() {
    free(victo_base_path);
    free(db_base_path);
    free(logs_base_path);
    free(auth_base_path);
}

char* get_collection_base_path(char* db) {
    // logWriter(LOG_DEBUG, "adaptor collection_base_path started");
    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    vt__append_to_string_builder(&resultSB, get_db_base_path());
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, db);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, COLLECTIONS);

    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        // logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    // logWriter(LOG_DEBUG, "adaptor collection_base_path completed");
    return result;
}

char* get_collection_full_path(const char* db, const char* collection) {
    // logWriter(LOG_DEBUG, "adaptor collection_full_path started");
    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    vt__append_to_string_builder(&resultSB, get_db_base_path());
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, db);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, COLLECTIONS);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, collection);

    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        // logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    // logWriter(LOG_DEBUG, "adaptor collection_full_path completed");
    return result;
}

char* get_vector_base_path(char* db, char* collection) {
    // logWriter(LOG_DEBUG, "adaptor vector_base_path started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    vt__append_to_string_builder(&resultSB, get_db_base_path());
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, db);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, COLLECTIONS);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, collection);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, VECTORS);

    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        // logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    // logWriter(LOG_DEBUG, "adaptor vector_base_path completed");
    return result;
}

char* get_vector_full_path(char* db, char* collection, char* hash) {
    // logWriter(LOG_DEBUG, "adaptor vector_base_path started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    vt__append_to_string_builder(&resultSB, get_db_base_path());
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, db);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, COLLECTIONS);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, collection);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, VECTORS);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, hash);
    vt__append_to_string_builder(&resultSB, VICTO_FILE_EXT);


    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        // logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    // logWriter(LOG_DEBUG, "adaptor vector_base_path completed");
    return result;
}

char* get_subscription_base_path(char* db, char* collection) {
    // logWriter(LOG_DEBUG, "adaptor vector_base_path started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    vt__append_to_string_builder(&resultSB, get_db_base_path());
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, db);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, COLLECTIONS);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, collection);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, SUBSCRIPTIONS);

    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        // logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    // logWriter(LOG_DEBUG, "adaptor vector_base_path completed");
    return result;
}

char* get_subscription_full_path(char* db, char* collection, char* hash) {
    // logWriter(LOG_DEBUG, "adaptor vector_base_path started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    vt__append_to_string_builder(&resultSB, get_db_base_path());
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, db);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, COLLECTIONS);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, collection);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, SUBSCRIPTIONS);
    vt__append_to_string_builder(&resultSB, "/");
    vt__append_to_string_builder(&resultSB, hash);
    vt__append_to_string_builder(&resultSB, SUSCRIP_FILE_EXT);


    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        // logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    // logWriter(LOG_DEBUG, "adaptor vector_base_path completed");
    return result;
}