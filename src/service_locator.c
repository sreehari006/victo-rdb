#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "commons/constants.h"
#include "engine/file/interface/file_io.h"
#include "engine/vector/interface/vector_io.h"
#include "engine/collection/interface/collection.h"
#include "engine/db/interface/db.h"
#include "engine/vector/interface/vector_math_private.h"

PutVectorRS putVectorSL(char* location, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite) {
    PutVectorRS rs;

    char filename[strlen(location) + strlen(hash) + strlen(VICTO_FILE_EXT) + 2];
    strcpy(filename, location);
    strcat(filename, "/");
    strcat(filename, hash);
    strcat(filename, VICTO_FILE_EXT);

    if(!overwrite) {
        if(fileExists(filename)) {
            rs.errCode = RECORD_EXIST_ERROR_CODE;
            rs.errMsg = RECORD_EXIST_ERROR_MSG;
            return rs;
        }
    }
    rs = putVector(filename, ai_model, hash, vdim, vp, is_normal);
    return rs;
}

GetVectorRS getVectorSL(char* location, char* hash) {
    
    char filename[strlen(location) + strlen(hash) + strlen(VICTO_FILE_EXT) + 2];
    strcpy(filename, location);
    strcat(filename, "/");
    strcat(filename, hash);
    strcat(filename, VICTO_FILE_EXT);

    return getVector(filename);
}

QueryVectorRSWrapper queryVectorSL(char* location, char* ai_model, int vdim, double* vp, QueryOptions queryOptions) {
     if(queryOptions.do_normal) {
        double* n_vp = normalize_vector(vdim, vp);

        if(n_vp == NULL) {
            free(n_vp);
            return queryVector(location, ai_model, vdim, vp, queryOptions);
        }

        double this_vp[vdim]; 
        for(int i=0; i<vdim; i++) {
            this_vp[i] = n_vp[i];
        }
        free(n_vp);
        return queryVector(location, ai_model, vdim, this_vp, queryOptions);
    } else {
        return queryVector(location, ai_model, vdim, vp, queryOptions);
    }
    
}

Response newDBSL(const char* location) {
    return newDB(location);
}

Response newCollectionSL(const char* location) {
    return newCollection(location);
}

Response deleteCollectionSL(const char* location) {
    return deleteCollection(location);
}

CountRS collectionCountSL(const char* location) {
    return collectionCount(location);
}

CollectionListRS collectionListSL(const char* location) {
    return collectionList(location);
}

CountRS vectorCountSL(char* location) {
    return vectorCount(location);
}

VectorListRS vectorListSL(const char* location) {
    return vectorList(location);
}

Response deleteVectorSL(char* location, char* hash) {
    Response rs;
    char filename[strlen(location) + strlen(hash) + strlen(VICTO_FILE_EXT) + 2];
    strcpy(filename, location);
    strcat(filename, "/");
    strcat(filename, hash);
    strcat(filename, VICTO_FILE_EXT);
    if(fileExists(filename)) {
        rs = deleteVector(filename);
    } else {
       rs.errCode = FILE_OPEN_ERROR_CODE;
       rs.errMsg = FILE_OPEN_ERROR_MSG;
    }

    return rs;
}

