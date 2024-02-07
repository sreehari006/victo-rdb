#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "../../ds/datastructures.h"
#include "../../commons/constants.h"

__attribute__((visibility("hidden"))) 
Response add_new_db(const char* location) {
    Response rs;

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


    return rs;
}
