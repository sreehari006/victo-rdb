#include "./includes/adaptor_proto.h"
#include "./includes/globals_proto.h"
#include "../../utils/json/includes/json_proto.h"
#include "../../utils/strings/includes/string_builder_proto.h"
#include "../../utils/uuid/includes/uuid_proto.h"
#include "../../commons/constants.h"
#include "../../utils/logs/includes/log_proto.h"
#include "../auth/includes/crypto_proto.h"
#include "../../registry/includes/db_config_sl_proto.h"
#include "../../registry/includes/db_ops_sl_proto.h"
#include "../../registry/includes/user_ops_sl_proto.h"
#include "../../utils/ds/includes/enums.h"

typedef struct {
    SubscribeTrigMsgNode* head;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} SubscribeTrigMsgQueue;

static SubscribeTrigMsgQueue subscribeTrigMsgQueue;

char* string_array_to_string(char** array) {
    vt__log_writer(LOG_DEBUG, "adaptor string_array_to_string started");

    size_t totalLength = 0;
    int count = 0;

    while(array[count] != NULL) {
        totalLength += strlen(array[count]);
        count++;
    }

    if(count <= 0) {
        vt__log_writer(LOG_WARN, "Array is empty");
        return NULL;
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

    vt__log_writer(LOG_DEBUG, "adaptor string_array_to_string completed");
    return resultCopy;
}

char* double_array_to_string(double* array, int size) {
    vt__log_writer(LOG_DEBUG, "adaptor double_array_to_string started");

    if(size <= 0) {
        vt__log_writer(LOG_WARN, "Array is empty");
        return NULL;
    }

    size_t totalLength = 0;
    
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

    vt__log_writer(LOG_DEBUG, "adaptor double_array_to_string completed");
    return resultCopy;
}

char* response_to_string(Response* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor response_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\"}");

    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor response_to_string completed");
    return result;
}

char* count_rs_to_string(CountRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor count_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);
    
    char count[20];
    snprintf(count, sizeof(count), "%d", rs->count);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\", \"count\": ");
    vt__append_to_string_builder(&resultSB, count);
    vt__append_to_string_builder(&resultSB, "}");

    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor count_rs_to_string completed");
    return result;
}

void init_subscribe_trig_queue() {
    vt__log_writer(LOG_DEBUG, "adaptor initSubscribeTrigQueue started");

    subscribeTrigMsgQueue.head = NULL;
    pthread_mutex_init(&subscribeTrigMsgQueue.mutex, NULL);
    pthread_cond_init(&subscribeTrigMsgQueue.cond, NULL);    

    vt__log_writer(LOG_DEBUG, "adaptor initSubscribeTrigQueue completed");
}

void enqueue_subscribe_trig_message(const char* db, const char* collection, const char* vectorHash, char* clientID) {
    vt__log_writer(LOG_DEBUG, "adaptor enqueueSubscribeTrigMessage started");

    SubscribeTrigMsgNode* newNode = (SubscribeTrigMsgNode*)malloc(sizeof(SubscribeTrigMsgNode));
    if (newNode == NULL) {
        vt__log_writer(LOG_ERROR, "Error while creating node for new subscribe trigger message");
        return;
    }

    newNode->db = strdup(db);
    newNode->collection = strdup(collection);
    newNode->vectorHash = strdup(vectorHash);
    newNode->clientID = strdup(clientID);
    newNode->next = NULL;
    
    pthread_mutex_lock(&subscribeTrigMsgQueue.mutex);

    if (subscribeTrigMsgQueue.head == NULL) {
        subscribeTrigMsgQueue.head = newNode;
    } else {
        SubscribeTrigMsgNode* current = subscribeTrigMsgQueue.head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }

    pthread_cond_signal(&subscribeTrigMsgQueue.cond);
    pthread_mutex_unlock(&subscribeTrigMsgQueue.mutex);

    vt__log_writer(LOG_DEBUG, "adaptor enqueueSubscribeTrigMessage completed");
}

SubscribeTrigMsgNode* dequeue_subscribe_trig_message() {
    vt__log_writer(LOG_DEBUG, "adaptor dequeueSubscribeTrigMessage started");

    SubscribeTrigMsgNode* messageNode = (SubscribeTrigMsgNode *)malloc(sizeof(SubscribeTrigMsgNode));
    pthread_mutex_lock(&subscribeTrigMsgQueue.mutex);

    while (subscribeTrigMsgQueue.head == NULL) {
        pthread_cond_wait(&subscribeTrigMsgQueue.cond, &subscribeTrigMsgQueue.mutex);
    }

    SubscribeTrigMsgNode* head = subscribeTrigMsgQueue.head;

    messageNode->db = strdup(head->db);
    messageNode->collection = strdup(head->collection);
    messageNode->vectorHash = strdup(head->vectorHash);
    messageNode->clientID = strdup(head->clientID);

    subscribeTrigMsgQueue.head = head->next;

    pthread_mutex_unlock(&subscribeTrigMsgQueue.mutex);
    free(head->db);
    free(head->collection);
    free(head->vectorHash);
    free(head->clientID);
    free(head);

    vt__log_writer(LOG_DEBUG, "adaptor dequeueSubscribeTrigMessage completed");
    return messageNode;
}

SubscriptionListNode* _query_subscription(SubscribeTrigMsgNode* subscribeTrigMsgNode) {
    vt__log_writer(LOG_DEBUG, "adaptor query_subscription started");
    SubscriptionListNode* subscriptionListNode = query_subscription_sl(subscribeTrigMsgNode->db, subscribeTrigMsgNode->collection, subscribeTrigMsgNode->vectorHash);
    
    vt__log_writer(LOG_DEBUG, "adaptor query_subscription completed");
    
    return subscriptionListNode;
}

void free_subscribe_trig_messag_queue() {
    vt__log_writer(LOG_DEBUG, "adaptor freeSubscribeTrigMessagQueue started");

    SubscribeTrigMsgNode* current = subscribeTrigMsgQueue.head;
    while (current != NULL) {
        SubscribeTrigMsgNode* nextNode = current->next;
        free(current->vectorHash);
        free(current);
        current = nextNode;
    }
    subscribeTrigMsgQueue.head = NULL; 

    pthread_mutex_destroy(&subscribeTrigMsgQueue.mutex);
    pthread_cond_destroy(&subscribeTrigMsgQueue.cond);

    vt__log_writer(LOG_DEBUG, "adaptor freeSubscribeTrigMessagQueue completed");
}

char* subscription_message(char* vector_hash, char* query_hash) {
    vt__log_writer(LOG_DEBUG, "adaptor subscription_message started");

    if(vector_hash == NULL || query_hash == NULL) {
        vt__log_writer(LOG_DEBUG, "vector hash or query hash is NULL");
        return NULL;
    }

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", 0);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, SUCESS_MSG);
    vt__append_to_string_builder(&resultSB, ", \"vector_hash\": \"");
    vt__append_to_string_builder(&resultSB, vector_hash);
    vt__append_to_string_builder(&resultSB, ", \"query_hash\": \"");
    vt__append_to_string_builder(&resultSB, query_hash);
    vt__append_to_string_builder(&resultSB, "\"");
    vt__append_to_string_builder(&resultSB, "}");
    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor subscription_message completed");
    return result;
}

char* collection_list_rs_to_string(CollectionListRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor collection_list_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\"");

    if(rs->errCode == 0) {
        vt__append_to_string_builder(&resultSB, ", \"collections\": [");
        if(rs->collections != NULL) {
            char* collections = string_array_to_string(rs->collections);
            vt__append_to_string_builder(&resultSB, collections);
            free(collections);
        }
        vt__append_to_string_builder(&resultSB, "]");
    }

    vt__append_to_string_builder(&resultSB, "}");
    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor collection_list_rs_to_string completed");
    return result;
}

char* vector_list_rs_to_string(VectorListRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor vector_list_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\"");

    if(rs->errCode == 0) {
        vt__append_to_string_builder(&resultSB, ", \"vectors\": [");
        if(rs->vectors != NULL) {
            char* vectors = string_array_to_string(rs->vectors);
            vt__append_to_string_builder(&resultSB, vectors);
            free(vectors);
        } 
        vt__append_to_string_builder(&resultSB, "]");
    }

    vt__append_to_string_builder(&resultSB, "}");

    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor vector_list_rs_to_string completed");
    return result;
}

char* subscription_list_rs_to_string(SubscriptionListRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor subscription_list_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\"");

    if(rs->errCode == 0) {
        vt__append_to_string_builder(&resultSB, ", \"subscriptions\": [");
        if(rs->subscriptions != NULL) {
            char* subscriptions = string_array_to_string(rs->subscriptions);
            vt__append_to_string_builder(&resultSB, subscriptions);
            free(subscriptions);
        } 
        vt__append_to_string_builder(&resultSB, "]");
    }

    vt__append_to_string_builder(&resultSB, "}");

    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor subscription_list_rs_to_string completed");
    return result;
}

char* vector_rs_to_string(GetVectorRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor vector_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\"");

    if(rs->errCode == 0) {
        Node node = rs->node;
        char* vp = double_array_to_string(node.vp, node.vdim);

        char dimension[20];
        snprintf(dimension, sizeof(dimension), "%d", node.vdim);

        vt__append_to_string_builder(&resultSB, ", \"ai_model\": \"");
        vt__append_to_string_builder(&resultSB, node.ai_model);
        vt__append_to_string_builder(&resultSB, "\", \"hash\": \"");
        vt__append_to_string_builder(&resultSB, node.hash);
        vt__append_to_string_builder(&resultSB, "\", \"is_normal\": \"");
        vt__append_to_string_builder(&resultSB, node.normal);
        vt__append_to_string_builder(&resultSB, "\", \"dimension\": ");
        vt__append_to_string_builder(&resultSB, dimension);
        vt__append_to_string_builder(&resultSB, ", \"vp\": [");
        vt__append_to_string_builder(&resultSB, vp);
        free(vp);
        vt__append_to_string_builder(&resultSB, "]");
    }

    vt__append_to_string_builder(&resultSB, "}");

    
    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor vector_rs_to_string completed");
    return result;
}

char* subscription_rs_to_string(GetSubscriptionRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor subscription_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\"");

    if(rs->errCode == 0) {
        SubscriptionNode node = rs->node;
        char* vp = double_array_to_string(node.vp, node.vdim);

        char dimension[20];
        snprintf(dimension, sizeof(dimension), "%d", node.vdim);

        vt__append_to_string_builder(&resultSB, ", \"client_id\": \"");
        vt__append_to_string_builder(&resultSB, node.client_id);
        vt__append_to_string_builder(&resultSB, "\", \"ai_model\": \"");
        vt__append_to_string_builder(&resultSB, node.ai_model);
        vt__append_to_string_builder(&resultSB, "\", \"hash\": \"");
        vt__append_to_string_builder(&resultSB, node.hash);
        vt__append_to_string_builder(&resultSB, "\", \"is_normal\": \"");
        vt__append_to_string_builder(&resultSB, node.normal);
        vt__append_to_string_builder(&resultSB, "\", \"dimension\": ");
        vt__append_to_string_builder(&resultSB, dimension);
        vt__append_to_string_builder(&resultSB, ", \"q_ops\": {");

        char k_value[20];
        snprintf(k_value, sizeof(k_value), "%f", node.query_value);
        char p_value[20];
        snprintf(p_value, sizeof(p_value), "%f", node.p_value);

        vt__append_to_string_builder(&resultSB, "\"vd_method\": \"");
        vt__append_to_string_builder(&resultSB, to_string_vd_method(node.vector_distance_method));
        vt__append_to_string_builder(&resultSB, "\", \"logical_op\": \"");
        vt__append_to_string_builder(&resultSB, to_string_logical_ops(node.query_logical_op));
        vt__append_to_string_builder(&resultSB, "\", \"k_value\": ");
        vt__append_to_string_builder(&resultSB, k_value);
        vt__append_to_string_builder(&resultSB, ", \"p_value\": ");
        vt__append_to_string_builder(&resultSB, p_value);
        vt__append_to_string_builder(&resultSB, "}");
        vt__append_to_string_builder(&resultSB, ", \"vp\": [");
        vt__append_to_string_builder(&resultSB, vp);
        free(vp);
        vt__append_to_string_builder(&resultSB, "]");
    }

    vt__append_to_string_builder(&resultSB, "}");

    
    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor subscription_rs_to_string completed");
    return result;
}

char* put_vector_rs_to_string(PutVectorRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor put_vector_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    if(rs->errCode == 0) {
        vt__append_to_string_builder(&resultSB, "\", \"hash\": \"");
        vt__append_to_string_builder(&resultSB, rs->hash);
    }
    vt__append_to_string_builder(&resultSB, "\"}");
    
    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor put_vector_rs_to_string completed");
    return result;
}

char* put_subscription_rs_to_string(PutSubscriptionRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor put_subscription_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    if(rs->errCode == 0) {
        vt__append_to_string_builder(&resultSB, "\", \"hash\": \"");
        vt__append_to_string_builder(&resultSB, rs->hash);
    }
    vt__append_to_string_builder(&resultSB, "\"}");
    
    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor put_subscription_rs_to_string completed");
    return result;
}

char* query_vector_rs_to_string(QueryVectorRS* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor query_vector_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    char errCode[5];
    snprintf(errCode, sizeof(errCode), "%d", rs->errCode);

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\"");

    char dimension[20];
    snprintf(dimension, sizeof(dimension), "%d", rs->vdim);

    char distance[20];
    snprintf(distance, sizeof(distance), "%f", rs->distance);

    vt__append_to_string_builder(&resultSB, ", \"ai_model\": \"");
    vt__append_to_string_builder(&resultSB, rs->ai_model);
    vt__append_to_string_builder(&resultSB, "\", \"hash\": \"");
    vt__append_to_string_builder(&resultSB, rs->hash);
    vt__append_to_string_builder(&resultSB, "\", \"is_normal\": \"");
    vt__append_to_string_builder(&resultSB, rs->normal);
    vt__append_to_string_builder(&resultSB, "\", \"dimension\": ");
    vt__append_to_string_builder(&resultSB, dimension);
    vt__append_to_string_builder(&resultSB, ", \"distance\": ");
    vt__append_to_string_builder(&resultSB, distance);   
    vt__append_to_string_builder(&resultSB, "}");

    
    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor query_vector_rs_to_string completed");
    return result;
}

char* query_vector_wrapper_rs_to_string(QueryVectorRSWrapper* rs) {
    vt__log_writer(LOG_DEBUG, "adaptor query_vector_wrapper_rs_to_string started");

    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

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

    vt__append_to_string_builder(&resultSB, "{\"code\": ");
    vt__append_to_string_builder(&resultSB, errCode);
    vt__append_to_string_builder(&resultSB, ", \"message\": \"");
    vt__append_to_string_builder(&resultSB, rs->errMsg);
    vt__append_to_string_builder(&resultSB, "\", \"query_count\": ");
    vt__append_to_string_builder(&resultSB, queryCount);
    vt__append_to_string_builder(&resultSB, ", \"fault_count\": ");
    vt__append_to_string_builder(&resultSB, faultCount);
    vt__append_to_string_builder(&resultSB, ", \"vectors\": [");
    for(int i=0; i<rs->queryCount; i++) {
        vt__append_to_string_builder(&resultSB, queryVectorRS[i]);
        if(i<rs->queryCount-1) {
            vt__append_to_string_builder(&resultSB, ", ");
        }
        free(queryVectorRS[i]);
    }
    
    vt__append_to_string_builder(&resultSB, "], \"fault\": [");
        for(int i=0; i<rs->faultCount; i++) {
        vt__append_to_string_builder(&resultSB, faultVectorRS[i]);
        if(i<rs->faultCount-1) {
            vt__append_to_string_builder(&resultSB, ", ");
        }
        free(faultVectorRS[i]);
    }
    vt__append_to_string_builder(&resultSB, "]}");
    
    char* result = strdup(resultSB.data);
    vt__free_string_builder(&resultSB);

    if (result == NULL) {
        vt__log_writer(LOG_ERROR, "Memory allocation failed for result");
    }

    vt__log_writer(LOG_DEBUG, "adaptor query_vector_wrapper_rs_to_string completed");
    return result;
}

Response _add_db(char* db) {
    vt__log_writer(LOG_DEBUG, "adaptor add_db started");

    Response rs = add_new_db_sl(db);
    
    vt__log_writer(LOG_DEBUG, "adaptor add_db completed");
    return rs;
}

Response _add_collection(char* db, char* collection) {
    vt__log_writer(LOG_DEBUG, "adaptor add_collection started");

    Response rs = add_new_collection_sl(db, collection);
    
    vt__log_writer(LOG_DEBUG, "adaptor add_collection completed");
    return rs;
}

Response _delete_collection(char* db, char* collection) {
    vt__log_writer(LOG_DEBUG, "adaptor delete_collection started");

    Response rs = delete_collection_sl(db, collection);
    
    vt__log_writer(LOG_DEBUG, "adaptor delete_collection completed");
    return rs;
}

CountRS _count_collection(char* db) {
    vt__log_writer(LOG_DEBUG, "adaptor count_collection started");

    CountRS rs = collection_count_sl(db);
    
    vt__log_writer(LOG_DEBUG, "adaptor count_collection completed");
    return rs;
}

CollectionListRS _list_collection(char* db) {
    vt__log_writer(LOG_DEBUG, "adaptor list_collection started");

    CollectionListRS rs = collection_list_sl(db);
    
    vt__log_writer(LOG_DEBUG, "adaptor list_collection completed");
    return rs;
}

Response _delete_vector(char* db, char* collection, char* hash) {
    vt__log_writer(LOG_DEBUG, "adaptor delete_vector started");

    Response rs = delete_vector_sl(db, collection, hash);
    
    vt__log_writer(LOG_DEBUG, "adaptor delete_vector completed");
    return rs;
}

CountRS _count_vector(char* db, char* collection) {
    vt__log_writer(LOG_DEBUG, "adaptor count_vector started");

    CountRS rs = vector_count_sl(db, collection);
    
    vt__log_writer(LOG_DEBUG, "adaptor count_vector completed");
    return rs;
}

CountRS _count_subscription(char* db, char* collection) {
    vt__log_writer(LOG_DEBUG, "adaptor count_subscription started");

    CountRS rs = subscription_count_sl(db, collection);
    
    vt__log_writer(LOG_DEBUG, "adaptor count_subscription completed");
    return rs;
}

VectorListRS _list_vector(char* db, char* collection) {
    vt__log_writer(LOG_DEBUG, "adaptor list_vector started");

    VectorListRS rs = vector_list_sl(db, collection);
    
    vt__log_writer(LOG_DEBUG, "adaptor list_vector completed");
    return rs;
}

SubscriptionListRS _list_subscription(char* db, char* collection) {
    vt__log_writer(LOG_DEBUG, "adaptor list_subscription started");

    SubscriptionListRS rs = subscription_list_sl(db, collection);
    
    vt__log_writer(LOG_DEBUG, "adaptor list_subscription completed");
    return rs;
}

GetVectorRS _get_vector(char* db, char* collection, char* hash) {
    vt__log_writer(LOG_DEBUG, "adaptor get_vector started");

    GetVectorRS rs = get_vector_sl(db, collection, hash);
    
    vt__log_writer(LOG_DEBUG, "adaptor get_vector completed");
    return rs;
}

GetSubscriptionRS _get_subscription(char* db, char* collection, char* hash) {
    vt__log_writer(LOG_DEBUG, "adaptor get_subscription started");

    GetSubscriptionRS rs = get_subscription_sl(db, collection, hash);
    
    vt__log_writer(LOG_DEBUG, "adaptor get_subscription completed");
    return rs;
}

Response _delete_subscription(char* db, char* collection, char* hash) {
    vt__log_writer(LOG_DEBUG, "adaptor get_subscription started");

    Response rs = unsubscribe_sl(db, collection, hash);
    
    vt__log_writer(LOG_DEBUG, "adaptor get_subscription completed");
    return rs;
}

PutVectorRS _add_vector(char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite) {
    vt__log_writer(LOG_DEBUG, "adaptor add_vector started");

    PutVectorRS rs = put_vector_sl(db, collection, ai_model, hash, vdim, vp, is_normal, overwrite);
    
    vt__log_writer(LOG_DEBUG, "adaptor add_vector completed");
    return rs;
}

PutSubscriptionRS _add_subscription(char* client_id, char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite, SubscriptionQueryOptions queryOptions) {
    vt__log_writer(LOG_DEBUG, "adaptor add_subscription started");

    PutSubscriptionRS rs = subscribe_sl(client_id, db, collection, ai_model, hash, vdim, vp, is_normal, overwrite, queryOptions);
    
    vt__log_writer(LOG_DEBUG, "adaptor add_subscription completed");
    return rs;
}

QueryVectorRSWrapper _query_vector(char* db, char* collection, char* ai_model, int vdim, double* vp, QueryOptions queryOptions) {
    vt__log_writer(LOG_DEBUG, "adaptor query_vector started");

    QueryVectorRSWrapper rs = query_vector_sl(db, collection, ai_model, vdim, vp, queryOptions);
    
    vt__log_writer(LOG_DEBUG, "adaptor query_vector completed");
    return rs;
}

Response _add_user(char* userName, char* password) {
    vt__log_writer(LOG_DEBUG, "adaptor add_user started");
    User* user = (User*)malloc(sizeof(User));

    char* uuid = vt__get_uuid();
    char* hashPassword = vt__sha256(password);
    strcpy(user->name, userName);
    strcpy(user->password, hashPassword);
    strcpy(user->uuid, uuid);
    user->user_access[USER_ACCESS_INDEX] = USER_ACCESS_CHANGE_PASS_SELF;
    user->user_access[DB_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;
    user->user_access[COLLECTION_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;
    user->user_access[VECTOR_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;
    user->user_access[SUBSCRIPTION_ACCESS_INDEX] = USER_ACCESS_FULL_ACCESS;
    user->status = USER_STATUS_ACTIVE;
    
    Response rs = add_user_sl(user);
    free(uuid);
    free(hashPassword);
    free(user);

    vt__log_writer(LOG_DEBUG, "adaptor add_user completed");

    return rs;
}


Response _change_password(char* uuid, char* user_name, char* current_pass, char* new_pass, bool self) {
    vt__log_writer(LOG_DEBUG, "adaptor _change_password started");
    Response rs = change_password_sl(uuid, user_name, current_pass, new_pass, self);
    vt__log_writer(LOG_DEBUG, "adaptor _change_password completed");
    return rs;
}

bool verifyAccess(char* op, char* obj, ClientInfo ClientInfo) {
    int access = true;

    if(strcmp(op,"add") == 0 && strcmp(obj, "db") == 0) {
        access = ClientInfo.user_access[DB_ACCESS_INDEX] & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"add") == 0 && strcmp(obj, "collection") == 0) {
        access = ClientInfo.user_access[COLLECTION_ACCESS_INDEX] & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"count") == 0 && strcmp(obj, "collection") == 0) {
        access = ClientInfo.user_access[COLLECTION_ACCESS_INDEX] & USER_ACCESS_COUNT_ACCESS;
    } else if(strcmp(op,"list") == 0 && strcmp(obj, "collection") == 0) {
        access = ClientInfo.user_access[COLLECTION_ACCESS_INDEX] & USER_ACCESS_LIST_ACCESS;
    } else if(strcmp(op,"delete") == 0 && strcmp(obj, "collection") == 0) {
        access = ClientInfo.user_access[COLLECTION_ACCESS_INDEX] & USER_ACCESS_DELETE_ACCESS;
    } else if(strcmp(op,"put") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.user_access[VECTOR_ACCESS_INDEX] & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"get") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.user_access[VECTOR_ACCESS_INDEX] & USER_ACCESS_READ_ACCESS;
    } else if(strcmp(op,"delete") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.user_access[VECTOR_ACCESS_INDEX] & USER_ACCESS_DELETE_ACCESS;
    } else if(strcmp(op,"count") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.user_access[VECTOR_ACCESS_INDEX] & USER_ACCESS_COUNT_ACCESS;
    } else if(strcmp(op,"list") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.user_access[VECTOR_ACCESS_INDEX] & USER_ACCESS_LIST_ACCESS;
    } else if(strcmp(op,"query") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.user_access[VECTOR_ACCESS_INDEX] & USER_ACCESS_READ_MULTIPLE_ACCESS;
    } else if(strcmp(op,"add") == 0 && strcmp(obj, "user") == 0) {
        access = ClientInfo.user_access[USER_ACCESS_INDEX] & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"ch_pass") == 0 && strcmp(obj, "user") == 0) {
        access = ClientInfo.user_access[USER_ACCESS_INDEX] & USER_ACCESS_CHANGE_PASS_OTHERS;
    } else if(strcmp(op,"ch_my_pass") == 0 && strcmp(obj, "user") == 0) {
        access = ClientInfo.user_access[USER_ACCESS_INDEX] & USER_ACCESS_CHANGE_PASS_SELF;
    } else if(strcmp(op,"add") == 0 && strcmp(obj, "subscription") == 0) {
        access = ClientInfo.user_access[SUBSCRIPTION_ACCESS_INDEX] & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"get") == 0 && strcmp(obj, "subscription") == 0) {
        access = ClientInfo.user_access[SUBSCRIPTION_ACCESS_INDEX] & USER_ACCESS_READ_ACCESS;
    } else if(strcmp(op,"list") == 0 && strcmp(obj, "subscription") == 0) {
        access = ClientInfo.user_access[SUBSCRIPTION_ACCESS_INDEX] & USER_ACCESS_LIST_ACCESS;
    } else if(strcmp(op,"count") == 0 && strcmp(obj, "subscription") == 0) {
        access = ClientInfo.user_access[SUBSCRIPTION_ACCESS_INDEX] & USER_ACCESS_COUNT_ACCESS;
    } else if(strcmp(op,"delete") == 0 && strcmp(obj, "subscription") == 0) {
        access = ClientInfo.user_access[SUBSCRIPTION_ACCESS_INDEX] & USER_ACCESS_DELETE_ACCESS;
    }

    free(op);
    free(obj);
    return access;
}

char* do_db_ops(char* threadUUID, char* payload, ClientInfo clientInfo) {    
    vt__log_writer(LOG_DEBUG, "adaptor do_db_ops started");

    char* result;

    vt__log_writer(LOG_DEBUG, "adaptor metadataSB while db operations");
    StringBuilder metadataSB;
    vt__init_string_builder(&metadataSB, 10);

    vt__log_writer(LOG_DEBUG, "adaptor errorSB while db operations");
    StringBuilder errorSB;
    vt__init_string_builder(&errorSB, 10);

    vt__log_writer(LOG_DEBUG, "adaptor resultSB while db operations");
    StringBuilder resultSB;
    vt__init_string_builder(&resultSB, 10);

    vt__log_writer(LOG_DEBUG, "adaptor clientResponseSB while db operations");
    StringBuilder clientResponseSB;
    vt__init_string_builder(&clientResponseSB, 10);

    vt__append_to_string_builder(&metadataSB, "\"metadata\": [");
    vt__append_to_string_builder(&metadataSB, "{\"response_id\": \"");
    char* responseID = strdup(threadUUID);
    vt__append_to_string_builder(&metadataSB, responseID);
    free(responseID);
    vt__append_to_string_builder(&metadataSB, "\"}");

    vt__append_to_string_builder(&errorSB, "\"error\": [");
    vt__append_to_string_builder(&resultSB, "\"result\": [");
    vt__append_to_string_builder(&clientResponseSB, "{");

    char* db = get_victo_base_path_sl();

    vt__log_writer(LOG_DEBUG, "Payload: ");
    vt__log_writer(LOG_DEBUG, payload);
    JsonNode* root = vt__load_json(payload);

    vt__log_writer(LOG_INFO, "Check whether the DB path and Payload is valid");
    if(db == NULL) {
        vt__log_writer(LOG_ERROR, "The base path provided for DB is NULL");
        vt__append_to_string_builder(&errorSB, "\"Invalid server state. Please restart the server with valid configuration.\"");     
    } else if(root == NULL) {
        vt__log_writer(LOG_WARN, "The query provided is not a valid JSON"); 
        vt__append_to_string_builder(&errorSB, "\"Invalid Query (JSON format)\"");     
    } else {
            vt__log_writer(LOG_INFO, "The provided DB path and Payload is valid");
            JsonNode* opNode =  vt__search_json(root, "op");
            JsonNode* objNode = vt__search_json(root, "obj");
            JsonNode* argsNode = vt__search_json(root, "args");
            
            char* op = (opNode != NULL) ? opNode->value : "empty";
            char* obj = (objNode != NULL) ? objNode->value : "empty";
            
            if(verifyAccess(strdup(op), strdup(obj), clientInfo)) {
                bool isError;
                if(strcmp(op, "add") == 0 && strcmp(obj, "user") == 0) {
                    vt__log_writer(LOG_INFO, "Begin add user");

                    JsonNode* nameNode = vt__search_json(argsNode, "name");
                    if(nameNode == NULL || nameNode->value == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: name or, name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: name\"");
                    } 

                    JsonNode* passwordNode = vt__search_json(argsNode, "password");
                    if(passwordNode == NULL || passwordNode->value == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: password or, password provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: password\"");
                    } 

                    if(!isError) {
                        Response rs = _add_user(nameNode->value, passwordNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if((strcmp(op, "ch_pass") == 0 || strcmp(op, "ch_my_pass") == 0)  && strcmp(obj, "user") == 0) {
                    vt__log_writer(LOG_INFO, "Begin add user");

                    JsonNode* nameNode = vt__search_json(argsNode, "name");
                    if(nameNode == NULL || nameNode->value == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: name or, name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: name\"");
                    } 

                    JsonNode* passwordNode = vt__search_json(argsNode, "password");
                    if(passwordNode == NULL || passwordNode->value == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: password or, password provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: password\"");
                    } 

                    JsonNode* newPasswordNode = vt__search_json(argsNode, "new_password");
                    if(newPasswordNode == NULL || newPasswordNode->value == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: new_password or, new_password provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: new_password\"");
                    } 

                    if(!isError) {
                        bool self = true;

                        if(strcmp(op, "ch_pass") == 0) {
                            self = false;    
                        }

                        Response rs = _change_password(clientInfo.client_id, nameNode->value, passwordNode->value, newPasswordNode->value, self); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "add") == 0 && strcmp(obj, "db") == 0) {
                    vt__log_writer(LOG_INFO, "Begin add db");

                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    if(!isError) {
                        Response rs = _add_db(dbNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "add") == 0 && strcmp(obj, "collection") == 0) {
                    vt__log_writer(LOG_INFO, "Begin add collection");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        Response rs = _add_collection(dbNode->value, collectionNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "delete") == 0 && strcmp(obj, "collection") == 0) {
                    vt__log_writer(LOG_INFO, "Begin delete collection");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        Response rs = _delete_collection(dbNode->value, collectionNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }
                    
                } else if(strcmp(op, "count") == 0 && strcmp(obj, "collection") == 0) {
                    vt__log_writer(LOG_INFO, "Begin count collection");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    }

                    if(!isError) {
                        CountRS rs = _count_collection(dbNode->value); 
                        char* result = count_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "list") == 0 && strcmp(obj, "collection") == 0) {
                    vt__log_writer(LOG_INFO, "Begin list collection");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    }

                    if(!isError) {
                        CollectionListRS rs = _list_collection(dbNode->value); 
                        char* result = collection_list_rs_to_string(&rs);
                        int i=0;
                        if(rs.collections != NULL) {
                            while(rs.collections[i] != NULL) {
                                free(rs.collections[i]);
                                i++;
                            }
                            free(rs.collections);
                        }
                        
                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "delete") == 0 && strcmp(obj, "vector") == 0) {
                    vt__log_writer(LOG_INFO, "Begin delete vector");
                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    JsonNode* hashNode = vt__search_json(argsNode, "hash");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(hashNode == NULL || hashNode->value == NULL || !vt__is_valid_obj_name(hashNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: vector (hash) or, vector (hash) provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: vector (hash), or vector (hash) provided is invalid\"");
                    } 

                    if(!isError) {
                        Response rs = _delete_vector(dbNode->value, collectionNode->value, hashNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "count") == 0 && strcmp(obj, "vector") == 0) {
                    vt__log_writer(LOG_INFO, "Begin count vector");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        CountRS rs = _count_vector(dbNode->value, collectionNode->value); 
                        char* result = count_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "list") == 0 && strcmp(obj, "vector") == 0) {
                    vt__log_writer(LOG_INFO, "Begin list vector");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        VectorListRS rs = _list_vector(dbNode->value, collectionNode->value); 
                        char* result = vector_list_rs_to_string(&rs);
                        int i=0;
                        if(rs.vectors != NULL) {
                            while(rs.vectors[i] != NULL) {
                                free(rs.vectors[i]);
                                i++;
                            }
                            free(rs.vectors);
                        }

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }                
                    }

                } else if(strcmp(op, "get") == 0 && strcmp(obj, "vector") == 0) {
                    vt__log_writer(LOG_INFO, "Begin get vector");
                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    JsonNode* hashNode = vt__search_json(argsNode, "hash");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(hashNode == NULL || hashNode->value == NULL || !vt__is_valid_obj_name(hashNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: vector (hash) or, vector (hash) provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: vector (hash), or vector (hash) provided is invalid\"");
                    } 
                    if(!isError) {
                        GetVectorRS rs = _get_vector(dbNode->value, collectionNode->value, hashNode->value); 
                        char* result = vector_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }          
                    }

                } else if(strcmp(op, "put") == 0 && strcmp(obj, "vector") == 0) {
                    vt__log_writer(LOG_INFO, "Begin put vector");

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    JsonNode* aiModelNode = vt__search_json(argsNode, "ai_model");
                    JsonNode* vdimNode = vt__search_json(argsNode, "vdim");
                    JsonNode* vpNode = vt__search_json(argsNode, "vp");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(aiModelNode == NULL || aiModelNode->value == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: ai_model");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: ai model (ai_model)\"");
                    } 

                    int vdim = 0;

                    if(vdimNode == NULL || vdimNode->value == NULL || !vt__is_valid_integer(vdimNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: vector dimension (vdim)");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: vector dimension (vdim)\"");
                    } else {
                        vdim = atoi(vdimNode->value);
                    }

                    double vp[vdim];
                    if(vpNode == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: vector points (vp)");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: vector points (vp)\"");
                    } else {
                        int i=0;
                        while(i<vdim && vpNode->children[i] != NULL) {
                            char* errptr;
                            vp[i] = strtod(vpNode->children[i]->value, &errptr);
                            
                            if (*errptr != '\0') {
                                vt__log_writer(LOG_WARN, "Invalid vector points: ");
                                vt__log_writer(LOG_WARN, errptr);
                                (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                                vt__append_to_string_builder(&errorSB, "\"Invalid vector points\"");
                                break;
                            }
                            i++;
                        }
                    }

                    if(!isError) {
                        JsonNode* isNormalNode = vt__search_json(argsNode, "is_normal");
                        JsonNode* overwriteNode = vt__search_json(argsNode, "overwrite");

                        bool isNormal = (isNormalNode != NULL && strcasecmp(isNormalNode->value, "true") == 0) ? true : false;
                        bool overwrite = (overwriteNode != NULL && strcasecmp(overwriteNode->value, "true") == 0) ? true : false;

                        char* hash = vt__get_uuid();

                        PutVectorRS rs = _add_vector(dbNode->value, collectionNode->value, aiModelNode->value, hash, vdim, vp, isNormal, overwrite); 
                        free(hash);

                        char* result = put_vector_rs_to_string(&rs);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added new node to subscribe queue");
                            if(is_subscription_enabled()) {
                                enqueue_subscribe_trig_message(dbNode->value, collectionNode->value, rs.hash, clientInfo.client_id);
                            } else {
                                vt__log_writer(LOG_DEBUG, "Subscription disabled");
                            }
                            
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }  
                        free(rs.errMsg);
                        free(rs.hash);
                    }

                } else if(strcmp(op, "query") == 0 && strcmp(obj, "vector") == 0) {
                    vt__log_writer(LOG_INFO, "Begin query vector");

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    JsonNode* aiModelNode = vt__search_json(argsNode, "ai_model");
                    JsonNode* vdimNode = vt__search_json(argsNode, "vdim");
                    JsonNode* vpNode = vt__search_json(argsNode, "vp");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 
                    
                    if(aiModelNode == NULL || aiModelNode->value == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: ai_model");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: ai model (ai_model)\"");
                    } 
                    
                    int vdim = 0; 
                    if(vdimNode == NULL || vdimNode->value == NULL || !vt__is_valid_integer(vdimNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: vector dimension (vdim)");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: vector dimension (vdim)\"");
                    } else {
                        vdim = atoi(vdimNode->value);
                    }
                    
                    if(vpNode == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: vector points (vp)");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: vector points (vp)\"");
                    } 
                    
                    double vp[vdim];
                    char* errptr;
                    if(!isError) { 
                        int i=0;
                        while(vpNode->children[i] != NULL && i<vdim) {
                            vp[i] = strtod(vpNode->children[i]->value, &errptr);
                            if (*errptr != '\0') {
                                vt__log_writer(LOG_WARN, "Invalid vector points");
                                vt__log_writer(LOG_WARN, errptr);
                                (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                                vt__append_to_string_builder(&errorSB, "\"Invalid vector points\"");
                                break;
                            }
                            i++;
                        }    
                    }
                
                    if(!isError) {       

                        QueryOptions queryOptions;
                        JsonNode* queryOptionsNode = vt__search_json(argsNode, "q_ops");
                        
                        JsonNode* vdMethodNode = vt__search_json(queryOptionsNode, "vd_method");
                        queryOptions.vector_distance_method = (vdMethodNode != NULL) ? to_int_vd_method(vdMethodNode->value) : 0;
                        
                        JsonNode* limitNode = vt__search_json(queryOptionsNode, "limit");
                        queryOptions.query_limit = (limitNode != NULL && vt__is_valid_integer(vdMethodNode->value)) ? atoi(limitNode->value): -99;

                        JsonNode* logicalOpNode = vt__search_json(queryOptionsNode, "logical_op");
                        queryOptions.query_logical_op = (logicalOpNode != NULL)? to_int_logical_ops(logicalOpNode->value) : 0;
                        
                        JsonNode* queryValueNode = vt__search_json(queryOptionsNode, "k_value");
                        double query_value = (queryValueNode != NULL) ? strtod(queryValueNode->value, &errptr): 0;
                        if (*errptr != '\0') {
                            query_value = 0;
                        }   
                        queryOptions.query_value = query_value;

                        JsonNode* includeFaultNode = vt__search_json(queryOptionsNode, "include_fault");
                        queryOptions.include_fault = (includeFaultNode != NULL && strcasecmp(includeFaultNode->value, "true") == 0) ? true : false;
                        
                        JsonNode* pValueNode = vt__search_json(queryOptionsNode, "p_value");
                        double p_value = pValueNode != NULL ? strtod(pValueNode->value, &errptr) : 0;
                        if (*errptr != '\0') {
                            p_value = 0;
                        }
                        queryOptions.p_value = p_value;

                        JsonNode* doNormalNode = vt__search_json(queryOptionsNode, "do_normal");
                        queryOptions.do_normal = (doNormalNode != NULL && strcasecmp(doNormalNode->value, "true")) == 0 ? true : false;
                        
                        JsonNode* orderNode = vt__search_json(queryOptionsNode, "order");
                        queryOptions.order = (orderNode != NULL && strcasecmp(orderNode->value, "true") == 0) ? true : false;

                        QueryVectorRSWrapper rs = _query_vector(dbNode->value, collectionNode->value, aiModelNode->value, vdim, vp, queryOptions); 
                        char* result = query_vector_wrapper_rs_to_string(&rs);
                        free(rs.errMsg);
                        free(rs.queryVectorRS);
                        if(rs.faultCount > 0) {
                            free(rs.faultVectorRS);
                        }

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        } 
                    }

                } else if(strcmp(op, "add") == 0 && strcmp(obj, "subscription") == 0) {
                    vt__log_writer(LOG_INFO, "Begin add subscription");

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    JsonNode* aiModelNode = vt__search_json(argsNode, "ai_model");
                    JsonNode* vdimNode = vt__search_json(argsNode, "vdim");
                    JsonNode* vpNode = vt__search_json(argsNode, "vp");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(aiModelNode == NULL || aiModelNode->value == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: ai_model");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: ai model (ai_model)\"");
                    } 

                    int vdim = 0;
                    if(vdimNode == NULL || vdimNode->value == NULL || !vt__is_valid_integer(vdimNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: vector dimension (vdim)");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: vector dimension (vdim)\"");
                    } else {
                        vdim = atoi(vdimNode->value);
                    }

                    if(vpNode == NULL) {
                        vt__log_writer(LOG_WARN, "Missing parameter: vector points (vp)");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: vector points (vp)\"");
                    } 

                    double vp[vdim];
                    char* errptr;
                    if(!isError) {
                        int i=0;
                        while(i<vdim && vpNode->children[i] != NULL) {
                            vp[i] = strtod(vpNode->children[i]->value, &errptr);
                            
                            if (*errptr != '\0') {
                                vt__log_writer(LOG_WARN, "Invalid vector points: ");
                                vt__log_writer(LOG_WARN, errptr);
                                (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                                vt__append_to_string_builder(&errorSB, "\"Invalid vector points\"");
                                break;
                            }
                            i++;
                        }
                    }

                    if(!isError) {
                        JsonNode* isNormalNode = vt__search_json(argsNode, "is_normal");
                        JsonNode* overwriteNode = vt__search_json(argsNode, "overwrite");

                        bool isNormal = (isNormalNode != NULL && strcasecmp(isNormalNode->value, "true") == 0) ? true : false;
                        bool overwrite = (overwriteNode != NULL && strcasecmp(overwriteNode->value, "true") == 0) ? true : false;

                        SubscriptionQueryOptions queryOptions;
                        JsonNode* queryOptionsNode = vt__search_json(argsNode, "q_ops");
                        
                        JsonNode* vdMethodNode = vt__search_json(queryOptionsNode, "vd_method");
                        queryOptions.vector_distance_method = (vdMethodNode != NULL) ? to_int_vd_method(vdMethodNode->value): 0;
                                               
                        JsonNode* logicalOpNode = vt__search_json(queryOptionsNode, "logical_op");
                        queryOptions.query_logical_op = (logicalOpNode != NULL)? to_int_logical_ops(logicalOpNode->value): 0;
                        
                        JsonNode* queryValueNode = vt__search_json(queryOptionsNode, "query_value");
                        double query_value = queryValueNode != NULL ? strtod(queryValueNode->value, &errptr): 0;
                        if (*errptr != '\0') {
                            query_value = 0;
                        }   
                        queryOptions.query_value = query_value;
                      
                        JsonNode* pValueNode = vt__search_json(queryOptionsNode, "p_value");
                        double p_value = pValueNode != NULL ? strtod(pValueNode->value, &errptr) : 0;
                        if (*errptr != '\0') {
                            p_value = 0;
                        }
                        queryOptions.p_value = p_value;

                        char* hash = vt__get_uuid();
                        PutSubscriptionRS rs = _add_subscription(clientInfo.client_id, dbNode->value, collectionNode->value, aiModelNode->value, hash, vdim, vp, isNormal, overwrite, queryOptions); 
                        free(hash);

                        char* result = put_subscription_rs_to_string(&rs);

                        if(result != NULL) {
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }  
                        free(rs.errMsg);
                    }

                } else if(strcmp(op, "get") == 0 && strcmp(obj, "subscription") == 0) {
                    vt__log_writer(LOG_INFO, "Begin get subscription");
                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    JsonNode* hashNode = vt__search_json(argsNode, "hash");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(hashNode == NULL || hashNode->value == NULL || !vt__is_valid_obj_name(hashNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: subscription (hash) or, subscription (hash) provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: subscription (hash), or subscription (hash) provided is invalid\"");
                    } 
                    if(!isError) {
                        GetSubscriptionRS rs = _get_subscription(dbNode->value, collectionNode->value, hashNode->value); 
                        char* result = subscription_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }          
                    }

                } else if(strcmp(op, "list") == 0 && strcmp(obj, "subscription") == 0) {
                    vt__log_writer(LOG_INFO, "Begin list subscription");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        SubscriptionListRS rs = _list_subscription(dbNode->value, collectionNode->value); 
                        char* result = subscription_list_rs_to_string(&rs);
                        int i=0;
                        if(rs.subscriptions != NULL) {
                            while(rs.subscriptions[i] != NULL) {
                                free(rs.subscriptions[i]);
                                i++;
                            }
                            free(rs.subscriptions);
                        }

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }                
                    }

                } else if(strcmp(op, "count") == 0 && strcmp(obj, "subscription") == 0) {
                    vt__log_writer(LOG_INFO, "Begin count subscription");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        CountRS rs = _count_subscription(dbNode->value, collectionNode->value); 
                        char* result = count_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "delete") == 0 && strcmp(obj, "subscription") == 0) {
                    vt__log_writer(LOG_INFO, "Begin get subscription");
                    JsonNode* collectionNode = vt__search_json(argsNode, "collection");
                    JsonNode* hashNode = vt__search_json(argsNode, "hash");
                    JsonNode* dbNode = vt__search_json(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !vt__is_valid_obj_name(dbNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !vt__is_valid_obj_name(collectionNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(hashNode == NULL || hashNode->value == NULL || !vt__is_valid_obj_name(hashNode->value)) {
                        vt__log_writer(LOG_WARN, "Missing parameter: subscription (hash) or, subscription (hash) provided is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Missing parameter: subscription (hash), or subscription (hash) provided is invalid\"");
                    } 
                    if(!isError) {
                        Response rs = _delete_subscription(dbNode->value, collectionNode->value, hashNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            vt__log_writer(LOG_INFO, "Added result to resultSB");
                            vt__append_to_string_builder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            vt__log_writer(LOG_ERROR, "DB Operation is unsuccessful");
                            vt__append_to_string_builder(&errorSB, "\"Internal server error\"");
                        }          
                    }

                } else {
                    if(strcmp(op, "empty") == 0) {
                        vt__log_writer(LOG_WARN, "Operation was missing in the payload");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Invalid Operation\"");
                    } else  if(strcmp(obj, "empty") == 0) {
                        vt__log_writer(LOG_WARN, "DB Object provided in the payload is invalid");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Operation on invalid Object\"");
                    } else {
                        vt__log_writer(LOG_WARN, "Operation and DB object is missing in the payload");
                        (isError) ? vt__append_to_string_builder(&errorSB, ", ") : (isError = true);
                        vt__append_to_string_builder(&errorSB, "\"Invalid query\"");            
                    }
                }

            } else {
                vt__log_writer(LOG_WARN, "Authorization failure");
                vt__append_to_string_builder(&errorSB, "\"Authorization failure\"");     
            }

            vt__free_json(root);
    }

    free(clientInfo.client_id);
    
    vt__append_to_string_builder(&metadataSB, "]");
    vt__append_to_string_builder(&clientResponseSB, metadataSB.data);
    vt__free_string_builder(&metadataSB);
    vt__log_writer(LOG_DEBUG, "Added metadata to client response");
    vt__append_to_string_builder(&clientResponseSB, ", ");
    
    vt__append_to_string_builder(&resultSB, "]");
    vt__append_to_string_builder(&clientResponseSB, resultSB.data);
    vt__free_string_builder(&resultSB);
    vt__log_writer(LOG_DEBUG, "Added db operation result to client response");
    vt__append_to_string_builder(&clientResponseSB, ", ");

    vt__append_to_string_builder(&errorSB, "]");
    vt__append_to_string_builder(&clientResponseSB, errorSB.data);
    vt__free_string_builder(&errorSB);
    vt__log_writer(LOG_DEBUG, "Added errors if any to client response");

    vt__append_to_string_builder(&clientResponseSB, "}");

    result = strdup(clientResponseSB.data);
    vt__free_string_builder(&clientResponseSB);
    
    vt__log_writer(LOG_DEBUG, "adaptor do_db_ops completed");
    return result;
}


