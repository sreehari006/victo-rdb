#include "interface/adaptor.h"
#include "../../utils/json/interface/json.h"
#include "../../service_locator.h"
#include "../../utils/strings/interface/string_builder.h"
#include "../../utils/uuid/interface/uuid.h"
#include "interface/globals.h"


char* string_array_to_string(char** array) {
    size_t totalLength = 0;
    int count = 0;

    while(array[count] != NULL) {
        totalLength += strlen(array[count]);
        count++;
    }

    totalLength += ((count*4) - 1);

    char result[totalLength];
    result[0] = '\0'; 

    for(int i=0; i<count; i++) {
        strcat(result, "\"");
        strcat(result, array[i]);
        strcat(result, "\"");
        if(i<(count-1)) {
            strcat(result, ", ");
        }
    }

    char* resultCopy = strdup(result);

    return resultCopy;
}

char* double_array_to_string(double* array, int size) {
    size_t totalLength = 0;
    int count = 0;

    totalLength = (size * 30) + size;

    char result[totalLength];
    result[0] = '\0'; 

    char vp[30];

    for(int i=0; i<size; i++) {
        memset(vp, 0, sizeof(vp));
        snprintf(vp, sizeof(vp), "%lf", array[i]);
        strcat(result, vp);
        if(i<(size-1)) {
            strcat(result, ", ");
        }
    }

    char* resultCopy = strdup(result);

    return resultCopy;
}

char* collection_full_path(char* db, char* collection) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    appendToStringBuilder(&resultSB, getDatabasePath());
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, db);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, collection);

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* db_full_path(char* db) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    appendToStringBuilder(&resultSB, getDatabasePath());
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, db);

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* response_to_string(Response* rs) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    appendToStringBuilder(&resultSB, "{\"code\": ");
    appendToStringBuilder(&resultSB, errCode);
    appendToStringBuilder(&resultSB, ", \"message\": \"");
    appendToStringBuilder(&resultSB, rs->errMsg);
    appendToStringBuilder(&resultSB, "\"}");

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* count_rs_to_string(CountRS* rs) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);
    
    char count[20];
    snprintf(count, sizeof(count), "%d", rs->count);

    appendToStringBuilder(&resultSB, "{\"code\": ");
    appendToStringBuilder(&resultSB, errCode);
    appendToStringBuilder(&resultSB, ", \"message\": \"");
    appendToStringBuilder(&resultSB, rs->errMsg);
    appendToStringBuilder(&resultSB, "\", \"count\": ");
    appendToStringBuilder(&resultSB, count);
    appendToStringBuilder(&resultSB, "}");

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* collection_list_rs_to_string(CollectionListRS* rs) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    appendToStringBuilder(&resultSB, "{\"code\": ");
    appendToStringBuilder(&resultSB, errCode);
    appendToStringBuilder(&resultSB, ", \"message\": \"");
    appendToStringBuilder(&resultSB, rs->errMsg);
    appendToStringBuilder(&resultSB, "\"");

    if(rs->errCode == 0) {
        char* collections = string_array_to_string(rs->collections);
        appendToStringBuilder(&resultSB, ", \"collections\": [");
        appendToStringBuilder(&resultSB, collections);
        free(collections);
        appendToStringBuilder(&resultSB, "]");
    }

    appendToStringBuilder(&resultSB, "}");
    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* vector_list_rs_to_string(VectorListRS* rs) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    appendToStringBuilder(&resultSB, "{\"code\": ");
    appendToStringBuilder(&resultSB, errCode);
    appendToStringBuilder(&resultSB, ", \"message\": \"");
    appendToStringBuilder(&resultSB, rs->errMsg);
    appendToStringBuilder(&resultSB, "\"");

    if(rs->errCode == 0) {
        char* vectors = string_array_to_string(rs->vectors);
        appendToStringBuilder(&resultSB, ", \"vectors\": [");
        appendToStringBuilder(&resultSB, vectors);
        free(vectors);
        appendToStringBuilder(&resultSB, "]");
    }

    appendToStringBuilder(&resultSB, "}");

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* vector_rs_to_string(GetVectorRS* rs) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    appendToStringBuilder(&resultSB, "{\"code\": ");
    appendToStringBuilder(&resultSB, errCode);
    appendToStringBuilder(&resultSB, ", \"message\": \"");
    appendToStringBuilder(&resultSB, rs->errMsg);
    appendToStringBuilder(&resultSB, "\"");

    if(rs->errCode == 0) {
        Node node = rs->node;
        char* vp = double_array_to_string(node.vp, node.vdim);

        char dimension[20];
        snprintf(dimension, sizeof(dimension), "%d", node.vdim);

        appendToStringBuilder(&resultSB, ", \"ai_model\": \"");
        appendToStringBuilder(&resultSB, node.ai_model);
        appendToStringBuilder(&resultSB, "\", \"hash\": \"");
        appendToStringBuilder(&resultSB, node.hash);
        appendToStringBuilder(&resultSB, "\", \"is_normal\": \"");
        appendToStringBuilder(&resultSB, node.normal);
        appendToStringBuilder(&resultSB, "\", \"dimension\": ");
        appendToStringBuilder(&resultSB, dimension);
        appendToStringBuilder(&resultSB, ", \"vp\": [");
        appendToStringBuilder(&resultSB, vp);
        free(vp);
        appendToStringBuilder(&resultSB, "]");
    }

    appendToStringBuilder(&resultSB, "}");

    
    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* put_vector_rs_to_string(PutVectorRS* rs) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    appendToStringBuilder(&resultSB, "{\"code\": ");
    appendToStringBuilder(&resultSB, errCode);
    appendToStringBuilder(&resultSB, ", \"message\": \"");
    appendToStringBuilder(&resultSB, rs->errMsg);
    if(rs->errCode == 0) {
        appendToStringBuilder(&resultSB, "\", \"hash\": \"");
        appendToStringBuilder(&resultSB, rs->hash);
    }
    appendToStringBuilder(&resultSB, "\"}");
    
    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* query_vector_rs_to_string(QueryVectorRS* rs) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    appendToStringBuilder(&resultSB, "{\"code\": ");
    appendToStringBuilder(&resultSB, errCode);
    appendToStringBuilder(&resultSB, ", \"message\": \"");
    appendToStringBuilder(&resultSB, rs->errMsg);
    appendToStringBuilder(&resultSB, "\"");

    char dimension[20];
    snprintf(dimension, sizeof(dimension), "%d", rs->vdim);

    char distance[20];
    snprintf(distance, sizeof(distance), "%f", rs->distance);

    appendToStringBuilder(&resultSB, ", \"ai_model\": \"");
    appendToStringBuilder(&resultSB, rs->ai_model);
    appendToStringBuilder(&resultSB, "\", \"hash\": \"");
    appendToStringBuilder(&resultSB, rs->hash);
    appendToStringBuilder(&resultSB, "\", \"is_normal\": \"");
    appendToStringBuilder(&resultSB, rs->normal);
    appendToStringBuilder(&resultSB, "\", \"dimension\": ");
    appendToStringBuilder(&resultSB, dimension);
    appendToStringBuilder(&resultSB, ", \"distance\": ");
    appendToStringBuilder(&resultSB, distance);   
    appendToStringBuilder(&resultSB, "}");

    
    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

char* query_vector_wrapper_rs_to_string(QueryVectorRSWrapper* rs) {
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    char queryCount[20];
    snprintf(queryCount, sizeof(queryCount), "%d", rs->queryCount);

    char faultCount[20];
    snprintf(faultCount, sizeof(faultCount), "%d", rs->faultCount);

    char* queryVectorRS[rs->queryCount];
    char* faultVectorRS[rs->faultCount];

    for(int i=0; i<rs->queryCount; i++) {
        queryVectorRS[i] = query_vector_rs_to_string(&rs->queryVectorRS[i]);
    }

    for(int i=0; i<rs->faultCount; i++) {
        faultVectorRS[i] = query_vector_rs_to_string(&rs->faultVectorRS[i]);
    }

    appendToStringBuilder(&resultSB, "{\"code\": ");
    appendToStringBuilder(&resultSB, errCode);
    appendToStringBuilder(&resultSB, ", \"message\": \"");
    appendToStringBuilder(&resultSB, rs->errMsg);
    appendToStringBuilder(&resultSB, "\", \"query_count\": ");
    appendToStringBuilder(&resultSB, queryCount);
    appendToStringBuilder(&resultSB, ", \"fault_count\": ");
    appendToStringBuilder(&resultSB, faultCount);
    appendToStringBuilder(&resultSB, ", \"vectors\": [");
    for(int i=0; i<rs->queryCount; i++) {
        appendToStringBuilder(&resultSB, queryVectorRS[i]);
        if(i<rs->queryCount-1) {
            appendToStringBuilder(&resultSB, ", ");
        }
        free(queryVectorRS[i]);
    }
    
    appendToStringBuilder(&resultSB, "], \"fault\": [");
        for(int i=0; i<rs->faultCount; i++) {
        appendToStringBuilder(&resultSB, faultVectorRS[i]);
        if(i<rs->faultCount-1) {
            appendToStringBuilder(&resultSB, ", ");
        }
        free(faultVectorRS[i]);
    }
    appendToStringBuilder(&resultSB, "]}");
    
    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    /* if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    } */
    return result;
}

Response add_db(char* db) {
    char* dbFP = db_full_path(db);
    Response rs = newDBSL(dbFP);
    free(dbFP);
    return rs;
}

Response add_collection(char* db, char* collection) {
    char* collectionFP = collection_full_path(db, collection);
    Response rs = newCollectionSL(collectionFP);
    free(collectionFP);
    return rs;
}

Response delete_collection(char* db, char* collection) {
    char* collectionFP = collection_full_path(db, collection);
    Response rs = deleteCollectionSL(collectionFP);
    free(collectionFP);
    return rs;
}

CountRS count_collection(char* db) {
    char* dbFP = db_full_path(db);
    CountRS rs = collectionCountSL(dbFP);
    free(dbFP);
    return rs;
}

CollectionListRS list_collection(char* db) {
    char* dbFP = db_full_path(db);
    CollectionListRS rs = collectionListSL(dbFP);
    free(dbFP);
    return rs;
}

Response delete_vector(char* db, char* collection, char* hash) {
    char* collectionFP = collection_full_path(db, collection);
    Response rs = deleteVectorSL(collectionFP, hash);
    free(collectionFP);
    return rs;
}

CountRS count_vector(char* db, char* collection) {
    char* collectionFP = collection_full_path(db, collection);
    CountRS rs = vectorCountSL(collectionFP);
    free(collectionFP);
    return rs;
}

VectorListRS list_vector(char* db, char* collection) {
    char* collectionFP = collection_full_path(db, collection);
    VectorListRS rs = vectorListSL(collectionFP);
    free(collectionFP);
    return rs;
}

GetVectorRS get_vector(char* db, char* collection, char* hash) {
    char* collectionFP = collection_full_path(db, collection);
    GetVectorRS rs = getVectorSL(collectionFP, hash);
    free(collectionFP);
    return rs;
}

PutVectorRS add_vector(char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite) {
    char* collectionFP = collection_full_path(db, collection);
    PutVectorRS rs = putVectorSL(collectionFP, ai_model, hash, vdim, vp, is_normal, overwrite);
    free(collectionFP);
    return rs;
}

QueryVectorRSWrapper query_vector(char* db, char* collection, char* ai_model, int vdim, double* vp, QueryOptions queryOptions) {
    char* collectionFP = collection_full_path(db, collection);
    QueryVectorRSWrapper rs = queryVectorSL(collectionFP, ai_model, vdim, vp, queryOptions);
    free(collectionFP);
    return rs;
}

char* do_db_ops(char* payload) {
    char* result;

    StringBuilder metadataSB;
    initStringBuilder(&metadataSB, 10);

    StringBuilder errorSB;
    initStringBuilder(&errorSB, 10);

    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    StringBuilder clientResponseSB;
    initStringBuilder(&clientResponseSB, 10);

    appendToStringBuilder(&metadataSB, "\"metadata\": [");
    appendToStringBuilder(&metadataSB, "{\"response_id\": \"");
    char* responseID = getUUID();
    appendToStringBuilder(&metadataSB, responseID);
    free(responseID);
    appendToStringBuilder(&metadataSB, "\"}");

    appendToStringBuilder(&errorSB, "\"error\": [");
    appendToStringBuilder(&resultSB, "\"result\": [");
    appendToStringBuilder(&clientResponseSB, "{");

    char* db = getDatabasePath();
    JsonNode* root = loadJson(payload);
    if(db == NULL) {
        appendToStringBuilder(&errorSB, "\"Invalid server state. Please restart the server with valid configuration.\"");     
    } else if(root == NULL) {
        appendToStringBuilder(&errorSB, "\"Invalid Query (JSON format)\"");     
    } else {
            JsonNode* opNode =  searchJson(root, "op");
            JsonNode* objNode = searchJson(root, "obj");
            JsonNode* argsNode = searchJson(root, "args");
            
            char* op = (opNode != NULL) ? opNode->value : "empty";
            char* obj = (objNode != NULL) ? objNode->value : "empty";
            
            bool isError;
            if(strcmp(op, "add") == 0 && strcmp(obj, "db") == 0) {
                JsonNode* dbNode = searchJson(argsNode, "db");
                printf("----- %d ----- \n", !isValidObjName(dbNode->value));
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 

                if(!isError) {
                    Response rs = add_db(dbNode->value); 
                    char* result = response_to_string(&rs);
                    free(rs.errMsg);
                    appendToStringBuilder(&resultSB, result);
                    free(result);
                }

            } else if(strcmp(op, "add") == 0 && strcmp(obj, "collection") == 0) {
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 

                JsonNode* collectionNode = searchJson(argsNode, "collection");
                if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                } 

                if(!isError) {
                    Response rs = add_collection(dbNode->value, collectionNode->value); 
                    char* result = response_to_string(&rs);
                    free(rs.errMsg);
                    appendToStringBuilder(&resultSB, result);
                    free(result);
                }

            } else if(strcmp(op, "delete") == 0 && strcmp(obj, "collection") == 0) {
                /* JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 

                JsonNode* collectionNode = searchJson(argsNode, "collection");
                
                if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                } 

                if(!isError) {
                    Response rs = delete_collection(dbNode->value, collectionNode->value); 
                    char* result = response_to_string(&rs);
                    free(rs.errMsg);
                    appendToStringBuilder(&resultSB, result);
                    free(result);
                } */
                
                //char* result = strdup("Operation not supported in this version");
                // return result;

                appendToStringBuilder(&resultSB, "\"Operation not supported in this version\"");

            } else if(strcmp(op, "count") == 0 && strcmp(obj, "collection") == 0) {
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                }

                if(!isError) {
                    CountRS rs = count_collection(dbNode->value); 
                    char* result = count_rs_to_string(&rs);
                    free(rs.errMsg);
                    appendToStringBuilder(&resultSB, result);
                    free(result);
                }

            } else if(strcmp(op, "list") == 0 && strcmp(obj, "collection") == 0) {
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                }

                if(!isError) {
                    CollectionListRS rs = list_collection(dbNode->value); 
                    char* result = collection_list_rs_to_string(&rs);
                    int i=0;
                    if(rs.collections != NULL) {
                        while(rs.collections[i] != NULL) {
                            free(rs.collections[i]);
                            i++;
                        }
                        free(rs.collections);
                    }
                    
                    appendToStringBuilder(&resultSB, result);
                    free(result);
                }

            } else if(strcmp(op, "delete") == 0 && strcmp(obj, "vector") == 0) {
                JsonNode* collectionNode = searchJson(argsNode, "collection");
                JsonNode* hashNode = searchJson(argsNode, "hash");
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 

                if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                } 

                if(hashNode == NULL || hashNode->value == NULL || !isValidObjName(hashNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: vector (hash), or vector (hash) provided is invalid\"");
                } 

                if(!isError) {
                    Response rs = delete_vector(dbNode->value, collectionNode->value, hashNode->value); 
                    char* result = response_to_string(&rs);
                    free(rs.errMsg);
                    appendToStringBuilder(&resultSB, result);
                    free(result);
                }

            } else if(strcmp(op, "count") == 0 && strcmp(obj, "vector") == 0) {
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 

                JsonNode* collectionNode = searchJson(argsNode, "collection");
                if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                } 

                if(!isError) {
                    CountRS rs = count_vector(dbNode->value, collectionNode->value); 
                    char* result = count_rs_to_string(&rs);
                    free(rs.errMsg);
                    appendToStringBuilder(&resultSB, result);
                    free(result);
                }

            } else if(strcmp(op, "list") == 0 && strcmp(obj, "vector") == 0) {
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 

                JsonNode* collectionNode = searchJson(argsNode, "collection");
                if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                } 

                if(!isError) {
                    VectorListRS rs = list_vector(dbNode->value, collectionNode->value); 
                    char* result = vector_list_rs_to_string(&rs);
                    int i=0;
                    if(rs.vectors != NULL) {
                        while(rs.vectors[i] != NULL) {
                            free(rs.vectors[i]);
                            i++;
                        }
                        free(rs.vectors);
                    }   
                    appendToStringBuilder(&resultSB, result);
                    free(result);                     
                }

            } else if(strcmp(op, "get") == 0 && strcmp(obj, "vector") == 0) {
                JsonNode* collectionNode = searchJson(argsNode, "collection");
                JsonNode* hashNode = searchJson(argsNode, "hash");
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 
                
                if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                } 

                if(hashNode == NULL || hashNode->value == NULL || !isValidObjName(hashNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: vector (hash), or vector (hash) provided is invalid\"");
                } 
                if(!isError) {
                    GetVectorRS rs = get_vector(dbNode->value, collectionNode->value, hashNode->value); 
                    char* result = vector_rs_to_string(&rs);
                    free(rs.errMsg);
                    appendToStringBuilder(&resultSB, result);
                    free(result);             
                }

            } else if(strcmp(op, "put") == 0 && strcmp(obj, "vector") == 0) {
                JsonNode* collectionNode = searchJson(argsNode, "collection");
                JsonNode* aiModelNode = searchJson(argsNode, "ai_model");
                JsonNode* vdimNode = searchJson(argsNode, "vdim");
                JsonNode* vpNode = searchJson(argsNode, "vp");
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 
                
                if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                } 

                if(aiModelNode == NULL || aiModelNode->value == NULL) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: ai model (ai_model)\"");
                } 

                int vdim = 0;

                if(vdimNode == NULL || vdimNode->value == NULL || !isValidInteger(vdimNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: vector dimension (vdim)\"");
                } else {
                    vdim = atoi(vdimNode->value);
                }

                double vp[vdim];
                if(vpNode == NULL) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: vector points (vp)\"");
                } else {
                    int i=0;
                    while(i<vdim && vpNode->children[i] != NULL) {
                        char* errptr;
                        vp[i] = strtod(vpNode->children[i]->value, &errptr);
                        
                        if (*errptr != '\0') {
                            (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                            appendToStringBuilder(&errorSB, "\"Invalid vector points\"");
                            break;
                        }
                        i++;
                    }
                }

                if(!isError) {
                    JsonNode* isNormalNode = searchJson(argsNode, "is_normal");
                    JsonNode* overwriteNode = searchJson(argsNode, "overwrite");

                    bool isNormal = (isNormalNode != NULL && strcasecmp(isNormalNode->value, "true") == 0) ? true : false;
                    bool overwrite = (overwriteNode != NULL && strcasecmp(overwriteNode->value, "true") == 0) ? true : false;

                    char* hash = getUUID();

                    PutVectorRS rs = add_vector(dbNode->value, collectionNode->value, aiModelNode->value, hash, vdim, vp, isNormal, overwrite); 
                    free(hash);

                    char* result = put_vector_rs_to_string(&rs);
                    free(rs.errMsg);
                    free(rs.hash);
                    appendToStringBuilder(&resultSB, result);
                    free(result);  
                }

            } else if(strcmp(op, "query") == 0 && strcmp(obj, "vector") == 0) {
                JsonNode* collectionNode = searchJson(argsNode, "collection");
                JsonNode* aiModelNode = searchJson(argsNode, "ai_model");
                JsonNode* vdimNode = searchJson(argsNode, "vdim");
                JsonNode* vpNode = searchJson(argsNode, "vp");
                JsonNode* dbNode = searchJson(argsNode, "db");
                
                if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                } 
                
                if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                } 
                
                if(aiModelNode == NULL || aiModelNode->value == NULL) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: ai model (ai_model)\"");
                } 
                
                int vdim = 0; 
                if(vdimNode == NULL || vdimNode->value == NULL || !isValidInteger(vdimNode->value)) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: vector dimension (vdim)\"");
                } else {
                    vdim = atoi(vdimNode->value);
                }
                
                if(vpNode == NULL) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Missing parameter: vector points (vp)\"");
                } 
                
                double vp[vdim];
                char* errptr;
                if(!isError) { 
                    int i=0;
                    while(vpNode->children[i] != NULL && i<vdim) {
                        vp[i] = strtod(vpNode->children[i]->value, &errptr);
                        if (*errptr != '\0') {
                            (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                            appendToStringBuilder(&errorSB, "\"Invalid vector points\"");
                            break;
                        }
                        i++;
                    }    
                }
            
                if(!isError) {       

                    QueryOptions queryOptions;
                    JsonNode* queryOptionsNode = searchJson(argsNode, "qOps");
                    
                    JsonNode* vdMethodNode = searchJson(queryOptionsNode, "vd_method");
                    queryOptions.vector_distance_method = (vdMethodNode != NULL && !isValidInteger(vdMethodNode->value)) ? atoi(vdMethodNode->value): 0;
                    
                    JsonNode* limitNode = searchJson(queryOptionsNode, "limit");
                    queryOptions.query_limit = (limitNode != NULL && !isValidInteger(vdMethodNode->value)) ? atoi(limitNode->value): -99;
                    
                    JsonNode* logicalOpNode = searchJson(queryOptionsNode, "logical_op");
                    queryOptions.query_logical_op = (logicalOpNode != NULL && !isValidInteger(vdMethodNode->value))? atoi(logicalOpNode->value): 0;
                    
                    JsonNode* queryValueNode = searchJson(queryOptionsNode, "k_value");
                    double query_value = (queryValueNode != NULL && !isValidInteger(vdMethodNode->value)) ? strtod(queryValueNode->value, &errptr): 0;
                    if (*errptr != '\0') {
                        query_value = 0;
                    }   
                    queryOptions.query_value = query_value;

                    JsonNode* includeFaultNode = searchJson(queryOptionsNode, "include_fault");
                    queryOptions.include_fault = (includeFaultNode != NULL && strcasecmp(includeFaultNode->value, "true") == 0) ? true : false;
                    
                    JsonNode* pValueNode = searchJson(queryOptionsNode, "p_value");
                    double p_value = pValueNode != NULL ? strtod(pValueNode->value, &errptr) : 0;
                    if (*errptr != '\0') {
                        p_value = 0;
                    }
                    queryOptions.p_value = p_value;

                    JsonNode* doNormalNode = searchJson(queryOptionsNode, "do_normal");
                    queryOptions.do_normal = (doNormalNode != NULL && strcasecmp(doNormalNode->value, "true")) == 0 ? true : false;
                    
                    JsonNode* orderNode = searchJson(queryOptionsNode, "order");
                    queryOptions.order = (orderNode != NULL && strcasecmp(orderNode->value, "true") == 0) ? true : false;

                    QueryVectorRSWrapper rs = query_vector(dbNode->value, collectionNode->value, aiModelNode->value, vdim, vp, queryOptions); 
                    char* result = query_vector_wrapper_rs_to_string(&rs);
                    free(rs.errMsg);
                    free(rs.queryVectorRS);
                    if(rs.faultCount > 0) {
                        free(rs.faultVectorRS);
                    }
                    appendToStringBuilder(&resultSB, result);
                    free(result);  
                }

            } else {
                if(strcmp(op, "empty") == 0) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Invalid Operation\"");
                } else  if(strcmp(obj, "empty") == 0) {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Operation on invalid Object\"");
                } else {
                    (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                    appendToStringBuilder(&errorSB, "\"Invalid query\"");            
                }
            }

            freeJson(root);
    }

    appendToStringBuilder(&metadataSB, "]");
    appendToStringBuilder(&clientResponseSB, metadataSB.data);
    freeStringBuilder(&metadataSB);
    appendToStringBuilder(&clientResponseSB, ", ");
    
    appendToStringBuilder(&resultSB, "]");
    appendToStringBuilder(&clientResponseSB, resultSB.data);
    freeStringBuilder(&resultSB);
    appendToStringBuilder(&clientResponseSB, ", ");

    appendToStringBuilder(&errorSB, "]");
    appendToStringBuilder(&clientResponseSB, errorSB.data);
    freeStringBuilder(&errorSB);
    
    appendToStringBuilder(&clientResponseSB, "}");

    result = strdup(clientResponseSB.data);
    freeStringBuilder(&clientResponseSB);
    
    return result;
}