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
        if(mkdir(location, 0777) == 0) {
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = SUCESS_MSG;
        } else {
            if(errno == EEXIST) {
                rs.errCode = DB_EXIST_ERROR_CODE;
                rs.errMsg = DB_EXIST_ERROR_MSG;
            } else {
                rs.errCode = DB_CREATE_FAILED_ERROR_CODE;
                rs.errMsg = DB_CREATE_FAILED_ERROR_MSG;
            }
        }
    #endif

    return rs;
}