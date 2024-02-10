#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include "../vector/includes/vector_math_proto.h"
#include "../file/includes/file_io_proto.h"
#include "../../commons/constants.h"
#include "../vector/includes/vector_io_local_proto.h"
#include "../base/includes/db_config_proto.h"
#include "../../utils/files/includes/file_utils_proto.h"

PutSubscriptionRS do_subscribe(char* client_id, char* filename, char* ai_model, char* hash, int vdim, double* vp, char* normal, SubscriptionQueryOptions queryOptions) {
    PutSubscriptionRS rs;
    SubscriptionNode node;


    strncpy(node.client_id, client_id, 37);
    strncpy(node.ai_model, ai_model, 64);
    strncpy(node.hash, hash, 64);
    strncpy(node.normal, normal, 2);
    node.vdim = vdim;
    node.vector_distance_method = queryOptions.vector_distance_method;
    node.query_value = queryOptions.query_value;
    node.query_logical_op = queryOptions.query_logical_op;
    node.p_value = queryOptions.p_value;

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
    if(fwrite(&node, sizeof(SubscriptionNode), 1, outfile) == 1) {
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

PutSubscriptionRS subscribe(char* client_id, char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite, SubscriptionQueryOptions queryOptions) {
    char* filename = get_subscription_full_path(db, collection, hash);
    if(!overwrite) {
        if(vt__file_exists(filename)) {
            PutSubscriptionRS rs;
            rs.errCode = RECORD_EXIST_ERROR_CODE;
            rs.errMsg = RECORD_EXIST_ERROR_MSG;
            return rs;
        }
    }

    if(is_normal) {
        double* n_vp = normalize_vector(vdim, vp);

        if(n_vp == NULL) {
            free(n_vp);
            return do_subscribe(client_id, filename, ai_model, hash, vdim, vp, "N", queryOptions);
        }

        double this_vp[vdim]; 
        for(int i=0; i<vdim; i++) {
            this_vp[i] = n_vp[i];
        }
        free(n_vp);
        return do_subscribe(client_id, filename, ai_model, hash, vdim, this_vp, "Y", queryOptions);
    } 
    
    return do_subscribe(client_id, filename, ai_model, hash, vdim, vp, "N", queryOptions);
    
}

GetSubscriptionRS get_subscription(char* db, char* collection, char* hash) {
    SubscriptionNode node;
    GetSubscriptionRS rs;

    char* filename = get_subscription_full_path(db, collection, hash);
    FILE* infile;
    infile = fopen(filename, "rb");
    if (infile == NULL) {
        // printf("\nError opening file\n");
        rs.errCode = FILE_OPEN_ERROR_CODE;
        rs.errMsg = strdup(FILE_OPEN_ERROR_MSG);
        return rs;
    }

    fread(&node, sizeof(SubscriptionNode), 1, infile);
    fclose(infile);

    rs.errCode = SUCCESS_CODE;
    rs.errMsg = strdup(SUCESS_MSG);
    rs.node = node;
    /* rs.node.queryOptions.vector_distance_method = node.queryOptions.vector_distance_method;
    rs.node.queryOptions.query_logical_op = node.queryOptions.query_logical_op;
    rs.node.queryOptions.query_value = node.queryOptions.query_value;
    rs.node.queryOptions.p_value = node.queryOptions.p_value; */

    // printf("%d %d %f %f \n", rs.node.queryOptions.vector_distance_method, rs.node.queryOptions.query_logical_op, rs.node.queryOptions.query_value, rs.node.queryOptions.p_value);
    

    free(filename);
    return rs;
} 

Response unsubscribe(char* db, char* collection, char* hash) {
    Response rs;

    char* filename = get_subscription_full_path(db, collection, hash);
    switch(delete_subscription(filename)) {
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

CountRS subscription_count(char* db, char* collection) {
    CountRS rs;

    char* location = get_subscription_base_path(db, collection);
    if(vt__dir_exists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.count = get_victo_files_count(location, SUSCRIP_FILE_EXT);
    } else {
        rs.errCode = DIR_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DIR_NOT_EXIST_ERROR_MSG);
        rs.count = -1;        
    }

    free(location);
    return rs;

}

SubscriptionListRS subscription_list(char* db, char* collection) {
    SubscriptionListRS rs;

    char* location = get_subscription_base_path(db, collection);
    
    if(vt__dir_exists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.subscriptions = list_files(location, FILE_NAME_WITHOUT_EXTENSION, SUSCRIP_FILE_EXT);
    } else {
        rs.errCode = DIR_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DIR_NOT_EXIST_ERROR_MSG);
        rs.subscriptions = NULL;
    }

    free(location);
    return rs;
}

SubscriptionListNode* query_subscription(char* db, char* collection, char* v_hash) {
    SubscriptionListNode* subscriptionMessageList;
    char* s_base_location = get_subscription_base_path(db, collection);
    char* filename = get_vector_full_path(db, collection, v_hash);
    GetVectorRS vectorRS = get_vector_local(filename);
    
    if(vectorRS.errCode == 0) {
        SubscriptionListRS listRs = subscription_list(db, collection);
        if(listRs.errCode == 0 && listRs.subscriptions) {
            int i=0; 
            while(listRs.subscriptions[i] != NULL) {
                GetSubscriptionRS subscriptionRS = get_subscription(db, collection, listRs.subscriptions[i]);
                if(subscriptionRS.errCode == 0) {
                    QueryOptions queryOptions;
                    queryOptions.query_logical_op = subscriptionRS.node.query_logical_op;
                    queryOptions.query_value = subscriptionRS.node.query_value;
                    queryOptions.vector_distance_method = subscriptionRS.node.vector_distance_method;
                    queryOptions.p_value = subscriptionRS.node.p_value;
                    queryOptions.query_limit = 0;
                    queryOptions.include_fault = false;
                    queryOptions.order = -99;
                    queryOptions.do_normal = false;

                    QueryVectorRS qvRS = convert_to_query_vector_from_query(vectorRS.node, subscriptionRS.node.ai_model, subscriptionRS.node.vdim, subscriptionRS.node.vp, queryOptions);
                    if(qvRS.errCode == 0) {
                        if(compare(queryOptions.query_logical_op, qvRS.distance, queryOptions.query_value)) {
                            SubscriptionMessageNode* message = (SubscriptionMessageNode *)malloc(sizeof(SubscriptionMessageNode));
                            message->vector_hash = strdup(vectorRS.node.hash);
                            message->query_hash = strdup(subscriptionRS.node.hash);
                            message->next = NULL;

                            if(subscriptionMessageList == NULL) {
                                subscriptionMessageList = (SubscriptionListNode *)malloc(sizeof(SubscriptionListNode));
                                subscriptionMessageList->client_id = strdup(subscriptionRS.node.client_id);
                                subscriptionMessageList->message = message;
                                subscriptionMessageList->next = NULL;
                            } else {
                                SubscriptionListNode* currentSubscriptionListNode = subscriptionMessageList;
                                bool existingClient = false;
                                while(currentSubscriptionListNode != NULL) {
                                    if(strcmp(currentSubscriptionListNode->client_id, subscriptionRS.node.client_id) == 0) {
                                        message->next = currentSubscriptionListNode->message;
                                        currentSubscriptionListNode->message = message;
                                        existingClient = true;
                                        break;
                                    }                                    
                                    currentSubscriptionListNode = currentSubscriptionListNode->next;
                                }

                                if(!existingClient) {
                                    SubscriptionListNode* newNode = (SubscriptionListNode *)malloc(sizeof(SubscriptionListNode));
                                    newNode->client_id = strdup(subscriptionRS.node.client_id);
                                    newNode->message = message;
                                    newNode->next = subscriptionMessageList;  
                                    subscriptionMessageList = newNode;                       
                                }
                            }
                            // printf("%s %s %s\n", vectorRS.node.hash, subscriptionRS.node.hash, subscriptionRS.node.client_id);
                        }
                    }
                }

                i++;
            }
        }

    }

    free(filename);
    free(s_base_location);
    return subscriptionMessageList;
    
}
