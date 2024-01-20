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
#include "../../ds/datastructures.h"
#include "../../commons/constants.h"
#include "../file/interface/file_io.h"

__attribute__((visibility("hidden"))) 
Response newDB(const char* location) {
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
        char collections[strlen(location) + strlen(COLLECTIONS) + 1];
        strcpy(collections, location);
        strcat(collections, "/");
        strcat(collections, COLLECTIONS);

        if(mkdir(location, 0777) == 0) {
            mkdir(collections, 0777);
            
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = strdup(SUCESS_MSG);
        } else {
            if(errno == EEXIST) {
                rs.errCode = DB_EXIST_ERROR_CODE;
                rs.errMsg = strdup(DB_EXIST_ERROR_MSG);
            } else {
                rs.errCode = DB_CREATE_FAILED_ERROR_CODE;
                rs.errMsg = strdup(DB_CREATE_FAILED_ERROR_MSG);
            }
        }
    #endif

    return rs;
}

__attribute__((visibility("hidden"))) 
Response initDBConfig(const char* location) {
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
        char dbPath[strlen(location) + strlen(DB) + 1];
        strcpy(dbPath, location);
        strcat(dbPath, "/");
        strcat(dbPath, DB);

        char logPath[strlen(location) + strlen(LOGS) + 1];
        strcpy(logPath, location);
        strcat(logPath, "/");
        strcat(logPath, LOGS);

        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);        
        if(dirExists(location)) {
            if(!dirExists(dbPath)) {
                if(mkdir(dbPath, 0777) != 0) {
                    rs.errCode = FAILED_CODE;
                    rs.errMsg = strdup(FAILED_MSG);
                }
            }

            if(!dirExists(logPath)) {
                if(mkdir(logPath, 0777) != 0) {
                    rs.errCode = FAILED_CODE;
                    rs.errMsg = strdup(FAILED_MSG);
                }
            }
        } else {
            rs.errCode = FAILED_CODE;
            rs.errMsg = strdup(FAILED_MSG);
        }
    #endif

    return rs;
}

__attribute__((visibility("hidden"))) 
char* getDBBasePath(const char* location) {
    char dbPath[strlen(location) + strlen(DB) + 1];
    strcpy(dbPath, location);
    strcat(dbPath, "/");
    strcat(dbPath, DB);

    return strdup(dbPath);
}

__attribute__((visibility("hidden"))) 
char* getDBLogPath(const char* location) {
    char logPath[strlen(location) + strlen(LOGS) + 1];
    strcpy(logPath, location);
    strcat(logPath, "/");
    strcat(logPath, LOGS);

    return strdup(logPath);
}