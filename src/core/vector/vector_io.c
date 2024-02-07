#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include "../../commons/constants.h"
#include "../../ds/datastructures.h"
#include "../base/includes/db_config_proto.h"
#include "./includes/vector_math_proto.h"
#include "../file/includes/file_io_proto.h"
#include "../../utils/strings/includes/string_builder_proto.h"
#include "../../utils/files/includes/file_utils_proto.h"

bool compare(int op, double v1, double v2) {
    switch(op) {
        case OPERATORS_LESS_THAN_OR_EQUAL_TO:
            return v1 <= v2;
        case OPERATORS_LESS_THAN:
            return v1 < v2;
        case OPERATORS_EQUAL_TO:
            return v1 == v2;
        case OPERATORS_GREATER_THAN:
            return v1 > v2;
        case OPERATORS_GREATER_THAN_OR_EQUAL_TO:
            return v1 >= v2;
    }

    return true;
}

void free_linked_list(QueryVectorLinkedList* head) {
    QueryVectorLinkedList* this_node = head;
    while(this_node != NULL) {
        QueryVectorLinkedList* temp = this_node;
        this_node = this_node->next;
        free(temp);
    }
}

VectorDistanceRS vector_distance(int vector_distance_method, int vdim, double* vp1, double* vp2, QueryOptions queryOptions) {
    VectorDistanceRS rs;
    switch(vector_distance_method) {
        case EUCLIDEAN_DISTANCE:
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = strdup(SUCESS_MSG);
            rs.distance = euclidean_distance(vdim, vp1, vp2);
            break;
        case COSINE_SIMILARITY:
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = strdup(SUCESS_MSG);
            rs.distance = cosine_similarity(vdim, vp1, vp2);
            break;
        case MANHATTAN_DISTANCE:
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = strdup(SUCESS_MSG);
            rs.distance = manhattan_distance(vdim, vp1, vp2);
            break;
        case MINKOWSKI_DISTANCE:
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = strdup(SUCESS_MSG);
            rs.distance = minskowski_distance(vdim, vp1, vp2, queryOptions.p_value);
            break;
        case DOT_PRODUCT:
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = strdup(SUCESS_MSG);
            rs.distance = dot_product(vdim, vp1, vp2);
            break;
        default:
            rs.errCode = VECTOR_DIST_METH_UNDEFINED_ERR;
            rs.errMsg = strdup(VECTOR_DIST_METH_UNDEFINED_MSG);

    }
    return rs;
}

void add_query_result_to_list(QueryVectorLinkedList** listHead, QueryVectorRS qvRS, bool is_valid, bool order) {
    QueryVectorLinkedList* new_node = malloc(sizeof(QueryVectorLinkedList));
    new_node->next = NULL;
    new_node->queryVectorRS = qvRS;
    
    if(is_valid) {
        QueryVectorLinkedList* curr = *listHead;
        QueryVectorLinkedList* prev = NULL;

        while(curr != NULL && (order == true ? curr->queryVectorRS.distance < new_node->queryVectorRS.distance : curr->queryVectorRS.distance > new_node->queryVectorRS.distance)) {
            prev = curr;
            curr = curr->next;
        }

        if(prev == NULL) {
            new_node->next = *listHead;
            *listHead = new_node;
        } else {
            prev->next = new_node;
            new_node->next = curr;
        }    
    } else { 
        new_node->next = *listHead;
        *listHead = new_node;
    }
}

QueryVectorRS convert_to_query_vector_from_query(Node node, char* ai_model, int vdim, double* vp, QueryOptions queryOptions) {
    QueryVectorRS qvRS;

    strncpy(qvRS.ai_model, node.ai_model, 64);
    strncpy(qvRS.hash, node.hash, 64);
    strncpy(qvRS.normal, node.normal, 2);
    qvRS.vdim = node.vdim;

    if(strcmp(ai_model, node.ai_model) == 0 && vdim == node.vdim) {
        VectorDistanceRS vdRS = vector_distance(queryOptions.vector_distance_method, vdim, vp, node.vp, queryOptions);
        if(vdRS.errCode == 0) {
            qvRS.distance = vdRS.distance;
            qvRS.errCode = vdRS.errCode;
            strncpy(qvRS.errMsg, vdRS.errMsg, 18);
        } else {
            qvRS.errCode = vdRS.errCode;
            strncpy(qvRS.errMsg, vdRS.errMsg, 18);
        }
        free(vdRS.errMsg);
    } else {
        qvRS.errCode = RECORD_MISMATCH_ERROR_CODE;
        strncpy(qvRS.errMsg, RECORD_MISMATCH_ERROR_MSG, 18);
    }

    return qvRS;
}

QueryVectorRS* convert_to_query_vector_from_list(QueryVectorLinkedList* listHead, int count, int query_limit) {
    QueryVectorLinkedList* this_node = listHead;
    
    QueryVectorRS* rs = (QueryVectorRS*) malloc(count * sizeof(QueryVectorRS));
    
    int i=0;
    while(this_node != NULL) {
        if(query_limit > 0 && i >= query_limit) {
            break;
        }

        rs[i].errCode = this_node->queryVectorRS.errCode;
        strncpy(rs[i].errMsg, this_node->queryVectorRS.errMsg, 18);
        strncpy(rs[i].ai_model, this_node->queryVectorRS.ai_model, 64);
        strncpy(rs[i].normal, this_node->queryVectorRS.normal, 2);
        strncpy(rs[i].hash, this_node->queryVectorRS.hash, 64);
        rs[i].vdim = this_node->queryVectorRS.vdim;
        rs[i].distance = this_node->queryVectorRS.distance;

        this_node = this_node->next;
        i++;
    }
    return rs;
}

PutVectorRS put_vector_local(char* filename, char* ai_model, char* hash, int vdim, double* vp, char* normal) {
    PutVectorRS rs;
    Node node;

    strncpy(node.ai_model, ai_model, 64);
    strncpy(node.hash, hash, 64);
    strncpy(node.normal, normal, 2);
    node.vdim = vdim;
    
    for (int i=0; i<vdim; i++) {
        node.vp[i] = vp[i];
    }

    FILE* outfile;
    outfile = fopen(filename, "wb");
    if (outfile == NULL) {
        rs.errCode = FILE_OPEN_ERROR_CODE;
        rs.errMsg = strdup(FILE_OPEN_ERROR_MSG);
        return rs;
    }
    if(fwrite(&node, sizeof(Node), 1, outfile) == 1) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.hash = strdup(node.hash);
        return rs;
    }

    rs.errCode = FILE_WRITE_ERROR_CODE;
    rs.errMsg = strdup(FILE_WRITE_ERROR_MSG);

    fclose(outfile);

    free(filename);
    return rs;
}

PutVectorRS put_vector(char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite) {
    char* filename = get_vector_full_path(db, collection, hash);
    if(!overwrite) {
        if(vt__file_exists(filename)) {
            PutVectorRS rs;
            rs.errCode = RECORD_EXIST_ERROR_CODE;
            rs.errMsg = RECORD_EXIST_ERROR_MSG;
            return rs;
        }
    }


    if(is_normal) {
        double* n_vp = normalize_vector(vdim, vp);

        if(n_vp == NULL) {
            free(n_vp);
            return put_vector_local(filename, ai_model, hash, vdim, vp, "N");
        }

        double this_vp[vdim]; 
        for(int i=0; i<vdim; i++) {
            this_vp[i] = n_vp[i];
        }
        free(n_vp);
        return put_vector_local(filename, ai_model, hash, vdim, this_vp, "Y");
    } 
    
    return put_vector_local(filename, ai_model, hash, vdim, vp, "N");
    
}

GetVectorRS get_vector(char* db, char* collection, char* hash) {
    Node node;
    GetVectorRS rs;

    char* filename = get_vector_full_path(db, collection, hash);
    FILE* infile;
    infile = fopen(filename, "rb");
    if (infile == NULL) {
        // printf("\nError opening file\n");
        rs.errCode = FILE_OPEN_ERROR_CODE;
        rs.errMsg = strdup(FILE_OPEN_ERROR_MSG);
        return rs;
    }

    fread(&node, sizeof(Node), 1, infile);
    fclose(infile);

    rs.errCode = SUCCESS_CODE;
    rs.errMsg = strdup(SUCESS_MSG);
    rs.node = node;
    
    free(filename);
    return rs;
} 

GetVectorRS get_vector_local(char* filename) {
    Node node;
    GetVectorRS rs;

    FILE* infile;
    infile = fopen(filename, "rb");
    if (infile == NULL) {
        // printf("\nError opening file\n");
        rs.errCode = FILE_OPEN_ERROR_CODE;
        rs.errMsg = strdup(FILE_OPEN_ERROR_MSG);
        return rs;
    }

    fread(&node, sizeof(Node), 1, infile);
    fclose(infile);

    rs.errCode = SUCCESS_CODE;
    rs.errMsg = strdup(SUCESS_MSG);
    rs.node = node;
    
    return rs;
} 

Response delete_vector(char* db, char* collection, char* hash) {
    Response rs;

    char* filename = get_vector_full_path(db, collection, hash);
    switch(delete_victo_file(filename)) {
        case 0:
            rs.errCode = SUCCESS_CODE;
            rs.errMsg = strdup(SUCESS_MSG);
            break;
        case 1:
            rs.errCode = VECTOR_DELETE_FAILED_ERROR_CODE;
            rs.errMsg = strdup(VECTOR_DELETE_FAILED_ERROR_MSG);
            break;
        case -1:
            rs.errCode = FILE_NOT_VICTO_ERROR_CODE;
            rs.errMsg = strdup(FILE_NOT_VICTO_ERROR_MSG);
            break;
    }

    free(filename);
    return rs;
}

CountRS vector_count(char* db, char* collection) {
    CountRS rs;

    char* location = get_vector_base_path(db, collection);
    if(vt__dir_exists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.count = get_victo_files_count(location, VICTO_FILE_EXT);
    } else {
        rs.errCode = DIR_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DIR_NOT_EXIST_ERROR_MSG);
        rs.count = -1;        
    }
    return rs;

}

VectorListRS vector_list(char* db, char* collection) {
    VectorListRS rs;
    
    char* location = get_vector_base_path(db, collection);
    if(vt__dir_exists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.vectors = list_files(location, FILE_NAME_WITHOUT_EXTENSION, VICTO_FILE_EXT);
    } else {
        rs.errCode = DIR_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DIR_NOT_EXIST_ERROR_MSG);
        rs.vectors = NULL;
    }

    free(location);
    return rs;
}

QueryVectorRSWrapper query_vector_local(char* db, char* collection, char* ai_model, int vdim, double* vp, QueryOptions queryOptions) {    
    QueryVectorRSWrapper rs;

    DIR *dir;
    char path[PATH_MAX];
    struct dirent *entry;
    char extension[] = VICTO_FILE_EXT;

    char* location = get_vector_base_path(db, collection);
    dir = opendir(location);
    if(dir == NULL) {
        rs.errCode = DIR_OPEN_ERROR_CODE;
        rs.errMsg = strdup(DIR_OPEN_ERROR_MSG);
        return rs;
    }

    QueryVectorLinkedList* queryNodesHead = NULL;
    QueryVectorLinkedList* faultNodesHead = NULL;
    int queryNodesCount = 0;
    int faultNodesCount = 0;

    char** vectors = list_files(location, FILE_FULL_PATH, VICTO_FILE_EXT);
    if(vectors != NULL) {
        for(int i=0; vectors[i] != NULL; i++) {
                bool fault = false;
                strncpy(path, vectors[i], PATH_MAX);
                GetVectorRS rvRS = get_vector_local(path);

                // Check if returns a valid vector node
                if(rvRS.errCode == 0) {
                    Node node = rvRS.node;
                    QueryVectorRS qvRS = convert_to_query_vector_from_query(node, ai_model, vdim, vp, queryOptions);
                    if(qvRS.errCode > 0) {
                        fault = true;
                    }

                    // Check if to include fault nodes in the result
                    if(fault) {
                        if(queryOptions.include_fault) {
                            add_query_result_to_list(&faultNodesHead, qvRS, false, queryOptions.order);
                            faultNodesCount++;
                        }
                    } else {
                        if(compare(queryOptions.query_logical_op, qvRS.distance, queryOptions.query_value)){
                            add_query_result_to_list(&queryNodesHead, qvRS, true, queryOptions.order);
                            queryNodesCount++;  
                        } 
                    }
                }
        }
    }

    // Iterate LinkedList
    rs.errCode = SUCCESS_CODE;
    rs.errMsg = strdup(SUCESS_MSG);

    if(queryOptions.query_limit > 0 && queryOptions.query_limit <= queryNodesCount) {
        rs.queryCount = queryOptions.query_limit;
    } else {
        rs.queryCount = queryNodesCount;
    }
    
    QueryVectorRS* tempQueryVectorRS = convert_to_query_vector_from_list(queryNodesHead, queryNodesCount, queryOptions.query_limit);
    rs.queryVectorRS = tempQueryVectorRS;
    

   if(queryOptions.include_fault) {
        if(queryOptions.query_limit > 0 && queryOptions.query_limit <= faultNodesCount) {
            rs.faultCount = queryOptions.query_limit;
        } else {
            rs.faultCount = faultNodesCount;
        }
        QueryVectorRS* tempQueryVectorRS = convert_to_query_vector_from_list(faultNodesHead, faultNodesCount, queryOptions.query_limit);
        rs.faultVectorRS = tempQueryVectorRS;
    }

    free_linked_list(queryNodesHead);
    free_linked_list(faultNodesHead);
    free(location);
    return rs;
}

QueryVectorRSWrapper query_vector(char* db, char* collection, char* ai_model, int vdim, double* vp, QueryOptions queryOptions) {
     if(queryOptions.do_normal) {
        double* n_vp = normalize_vector(vdim, vp);

        if(n_vp == NULL) {
            free(n_vp);
            return query_vector_local(db, collection, ai_model, vdim, vp, queryOptions);
        }

        double this_vp[vdim]; 
        for(int i=0; i<vdim; i++) {
            this_vp[i] = n_vp[i];
        }
        free(n_vp);
        return query_vector_local(db, collection, ai_model, vdim, this_vp, queryOptions);
    } else {
        return query_vector_local(db, collection, ai_model, vdim, vp, queryOptions);
    }
}

