#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include "./interface/subscription.h"
#include "../vector/interface/vector_math_private.h"
#include "../file/interface/file_io.h"
#include "../../commons/constants.h"
#include "../vector/interface/vector_io_local.h"

__attribute__((visibility("hidden"))) 
PutSubscriptionRS do_subscribe(char* client_id, char* filename, char* ai_model, char* hash, int vdim, double* vp, char* normal, SubscriptionQueryOptions queryOptions) {
    PutSubscriptionRS rs;
    SubscriptionNode node;


    strncpy(node.client_id, client_id, 37);
    strncpy(node.ai_model, ai_model, 64);
    strncpy(node.hash, hash, 64);
    strncpy(node.normal, normal, 2);
    node.vdim = vdim;
    node.queryOptions.vector_distance_method = queryOptions.vector_distance_method;
    node.queryOptions.query_value = queryOptions.query_value;
    node.queryOptions.query_logical_op = queryOptions.query_logical_op;
    node.queryOptions.p_value = queryOptions.p_value;
    
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

    return rs;
}

__attribute__((visibility("hidden"))) 
PutSubscriptionRS subscribe(char* client_id, char* filename, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, SubscriptionQueryOptions queryOptions) {
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
        return do_subscribe(client_id,filename, ai_model, hash, vdim, this_vp, "Y", queryOptions);
    } 
    
    return do_subscribe(client_id, filename, ai_model, hash, vdim, vp, "N", queryOptions);
    
}

__attribute__((visibility("hidden"))) 
GetSubscriptionRS getSubscription(char* filename) {
    SubscriptionNode node;
    GetSubscriptionRS rs;

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
    
    return rs;
} 

__attribute__((visibility("hidden"))) 
Response unsubscribe(char* filename) {
    Response rs;
    switch(deleteSubscriptionFile(filename)) {
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

    return rs;
}
__attribute__((visibility("hidden"))) 
CountRS subscriptionCount(char* location) {
    CountRS rs;
    if(dirExists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.count = get_victo_files_count(location, SUSCRIP_FILE_EXT);
    } else {
        rs.errCode = DIR_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DIR_NOT_EXIST_ERROR_MSG);
        rs.count = -1;        
    }
    return rs;

}

__attribute__((visibility("hidden"))) 
SubscriptionListRS subscriptionList(const char* location) {
    SubscriptionListRS rs;
    if(dirExists(location)) {
        rs.errCode = SUCCESS_CODE;
        rs.errMsg = strdup(SUCESS_MSG);
        rs.subscriptions = list_files(location, FILE_NAME_WITHOUT_EXTENSION, SUSCRIP_FILE_EXT);
    } else {
        rs.errCode = DIR_NOT_EXIST_ERROR_CODE;
        rs.errMsg = strdup(DIR_NOT_EXIST_ERROR_MSG);
        rs.subscriptions = NULL;
    }
    return rs;
}

void querySubscription(char* s_base_location, char* filename) {
    GetVectorRS vectorRS = getVector(filename);
    if(vectorRS.errCode == 0) {
        SubscriptionListRS listRs = subscriptionList(s_base_location);
        if(listRs.errCode == 0) {
            int i=0; 
            while(listRs.subscriptions[i] != NULL) {
                char subscription[strlen(s_base_location) + strlen(listRs.subscriptions[i]) + strlen(SUSCRIP_FILE_EXT) + 2];
                strcpy(subscription, s_base_location);
                strcat(subscription, "/");
                strcat(subscription, listRs.subscriptions[i]);
                strcat(subscription, SUSCRIP_FILE_EXT);

                GetSubscriptionRS subscriptionRS = getSubscription(subscription);
                if(subscriptionRS.errCode == 0) {
                    QueryOptions queryOptions;
                    queryOptions.query_logical_op = subscriptionRS.node.queryOptions.query_logical_op;
                    queryOptions.query_value = subscriptionRS.node.queryOptions.query_value;
                    queryOptions.vector_distance_method = subscriptionRS.node.queryOptions.vector_distance_method;
                    queryOptions.p_value = subscriptionRS.node.queryOptions.p_value;
                    queryOptions.query_limit = 0;
                    queryOptions.include_fault = false;
                    queryOptions.order = -99;
                    queryOptions.do_normal = false;

                    QueryVectorRS qvRS = convert_to_query_vector_from_query(vectorRS.node, subscriptionRS.node.ai_model, subscriptionRS.node.vdim, subscriptionRS.node.vp, queryOptions);
                    if(qvRS.errCode == 0) {
                        printf("%s %f %d %f\n", qvRS.hash, qvRS.distance, queryOptions.query_logical_op, queryOptions.query_value);
                    }
                }

                i++;
            }
        }

    }
}
