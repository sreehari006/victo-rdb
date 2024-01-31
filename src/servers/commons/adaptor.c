#include "interface/adaptor.h"
#include "interface/globals.h"
#include "../../utils/json/interface/json.h"
#include "../../sl/db/interface/service_locator.h"
#include "../../utils/strings/interface/string_builder.h"
#include "../../utils/uuid/interface/uuid.h"
#include "../../commons/constants.h"
#include "../../utils/logs/interface/log.h"
#include "../auth/interface/user_ops.h"
#include "../auth/interface/crypto.h"

typedef struct SubscribeTrigMsgNode {
    char* vectorHash;
    char* clientID;
    struct SubscribeTrigMsgNode* next;
} SubscribeTrigMsgNode;

typedef struct {
    SubscribeTrigMsgNode* head;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} SubscribeTrigMsgQueue;

static SubscribeTrigMsgQueue subscribeTrigMsgQueue;

void initSubscribeTrigQueue() {
    logWriter(LOG_DEBUG, "adaptor initSubscribeTrigQueue started");

    subscribeTrigMsgQueue.head = NULL;
    pthread_mutex_init(&subscribeTrigMsgQueue.mutex, NULL);
    pthread_cond_init(&subscribeTrigMsgQueue.cond, NULL);    

    logWriter(LOG_DEBUG, "adaptor initSubscribeTrigQueue completed");
}

void enqueueSubscribeTrigMessage(const char* vectorHash, const char* clientID) {
    logWriter(LOG_DEBUG, "adaptor enqueueSubscribeTrigMessage started");

    SubscribeTrigMsgNode* newNode = (SubscribeTrigMsgNode*)malloc(sizeof(SubscribeTrigMsgNode));
    if (newNode == NULL) {
        logWriter(LOG_ERROR, "Error while creating node for new subscribe trigger message");
        return;
    }

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

    logWriter(LOG_DEBUG, "adaptor enqueueSubscribeTrigMessage completed");
}

SubscribeTrigMsgNode dequeueSubscribeTrigMessage() {
    logWriter(LOG_DEBUG, "adaptor dequeueSubscribeTrigMessage started");

    SubscribeReplyInfo SubscribeReplyInfo;
    SubscribeTrigMsgNode messageNode;
    pthread_mutex_lock(&subscribeTrigMsgQueue.mutex);

    while (subscribeTrigMsgQueue.head == NULL) {
        pthread_cond_wait(&subscribeTrigMsgQueue.cond, &subscribeTrigMsgQueue.mutex);
    }

    SubscribeTrigMsgNode* head = subscribeTrigMsgQueue.head;
    messageNode.vectorHash = strdup(head->vectorHash);
    messageNode.clientID = strdup(head->clientID);
    
    subscribeTrigMsgQueue.head = head->next;

    pthread_mutex_unlock(&subscribeTrigMsgQueue.mutex);
    free(head->vectorHash);
    free(head->clientID);
    free(head);

    logWriter(LOG_DEBUG, "adaptor dequeueSubscribeTrigMessage completed");
    return messageNode;
}

SubscribeReplyInfo querySubscription() {
    SubscribeReplyInfo subscribeReplyInfo;

    SubscribeTrigMsgNode node = dequeueSubscribeTrigMessage();
    subscribeReplyInfo.client_id = node.clientID;
    subscribeReplyInfo.vector_hash = node.vectorHash;

    return subscribeReplyInfo;
}

void freeSubscribeTrigMessagQueue() {
    logWriter(LOG_DEBUG, "adaptor freeSubscribeTrigMessagQueue started");

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

    logWriter(LOG_DEBUG, "adaptor freeSubscribeTrigMessagQueue completed");
}

char* string_array_to_string(char** array) {
    logWriter(LOG_DEBUG, "adaptor string_array_to_string started");

    size_t totalLength = 0;
    int count = 0;

    while(array[count] != NULL) {
        totalLength += strlen(array[count]);
        count++;
    }

    if(count <= 0) {
        logWriter(LOG_WARN, "Array is empty");
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

    logWriter(LOG_DEBUG, "adaptor string_array_to_string completed");
    return resultCopy;
}

char* double_array_to_string(double* array, int size) {
    logWriter(LOG_DEBUG, "adaptor double_array_to_string started");

    if(size <= 0) {
        logWriter(LOG_WARN, "Array is empty");
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

    logWriter(LOG_DEBUG, "adaptor double_array_to_string completed");
    return resultCopy;
}

char* db_full_path(char* db) {
    logWriter(LOG_DEBUG, "adaptor db_full_path started");

    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    appendToStringBuilder(&resultSB, getDatabasePath());
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, db);

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor db_full_path completed");
    return result;
}

char* collection_base_path(char* db) {
    logWriter(LOG_DEBUG, "adaptor collection_base_path started");
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    appendToStringBuilder(&resultSB, getDatabasePath());
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, db);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, COLLECTIONS);

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor collection_base_path completed");
    return result;
}

char* collection_full_path(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor collection_full_path started");
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    appendToStringBuilder(&resultSB, getDatabasePath());
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, db);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, COLLECTIONS);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, collection);

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor collection_full_path completed");
    return result;
}

char* vector_base_path(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor vector_base_path started");

    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    appendToStringBuilder(&resultSB, getDatabasePath());
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, db);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, COLLECTIONS);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, collection);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, VECTORS);

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor vector_base_path completed");
    return result;
}

char* subscription_base_path(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor subscription_base_path started");

    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    appendToStringBuilder(&resultSB, getDatabasePath());
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, db);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, COLLECTIONS);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, collection);
    appendToStringBuilder(&resultSB, "/");
    appendToStringBuilder(&resultSB, SUBSCRIPTIONS);

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor subscription_base_path completed");
    return result;
}

char* response_to_string(Response* rs) {
    logWriter(LOG_DEBUG, "adaptor response_to_string started");

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
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor response_to_string completed");
    return result;
}

char* count_rs_to_string(CountRS* rs) {
    logWriter(LOG_DEBUG, "adaptor count_rs_to_string started");

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
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor count_rs_to_string completed");
    return result;
}

char* collection_list_rs_to_string(CollectionListRS* rs) {
    logWriter(LOG_DEBUG, "adaptor collection_list_rs_to_string started");

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
        appendToStringBuilder(&resultSB, ", \"collections\": [");
        if(rs->collections != NULL) {
            char* collections = string_array_to_string(rs->collections);
            appendToStringBuilder(&resultSB, collections);
            free(collections);
        }
        appendToStringBuilder(&resultSB, "]");
    }

    appendToStringBuilder(&resultSB, "}");
    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor collection_list_rs_to_string completed");
    return result;
}

char* vector_list_rs_to_string(VectorListRS* rs) {
    logWriter(LOG_DEBUG, "adaptor vector_list_rs_to_string started");

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
        appendToStringBuilder(&resultSB, ", \"vectors\": [");
        if(rs->vectors != NULL) {
            char* vectors = string_array_to_string(rs->vectors);
            appendToStringBuilder(&resultSB, vectors);
            free(vectors);
        } 
        appendToStringBuilder(&resultSB, "]");
    }

    appendToStringBuilder(&resultSB, "}");

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor vector_list_rs_to_string completed");
    return result;
}

char* subscription_list_rs_to_string(SubscriptionListRS* rs) {
    logWriter(LOG_DEBUG, "adaptor subscription_list_rs_to_string started");

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
        appendToStringBuilder(&resultSB, ", \"subscriptions\": [");
        if(rs->subscriptions != NULL) {
            char* subscriptions = string_array_to_string(rs->subscriptions);
            appendToStringBuilder(&resultSB, subscriptions);
            free(subscriptions);
        } 
        appendToStringBuilder(&resultSB, "]");
    }

    appendToStringBuilder(&resultSB, "}");

    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor subscription_list_rs_to_string completed");
    return result;
}

char* vector_rs_to_string(GetVectorRS* rs) {
    logWriter(LOG_DEBUG, "adaptor vector_rs_to_string started");

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
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor vector_rs_to_string completed");
    return result;
}

char* subscription_rs_to_string(GetSubscriptionRS* rs) {
    logWriter(LOG_DEBUG, "adaptor subscription_rs_to_string started");

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
        SubscriptionNode node = rs->node;
        char* vp = double_array_to_string(node.vp, node.vdim);

        char dimension[20];
        snprintf(dimension, sizeof(dimension), "%d", node.vdim);

        appendToStringBuilder(&resultSB, ", \"client_id\": \"");
        appendToStringBuilder(&resultSB, node.client_id);
        appendToStringBuilder(&resultSB, "\", \"ai_model\": \"");
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
        appendToStringBuilder(&resultSB, ", \"query_options\": [");
        appendToStringBuilder(&resultSB, "]");
    }

    appendToStringBuilder(&resultSB, "}");

    
    char* result = strdup(resultSB.data);
    freeStringBuilder(&resultSB);

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor subscription_rs_to_string completed");
    return result;
}

char* put_vector_rs_to_string(PutVectorRS* rs) {
    logWriter(LOG_DEBUG, "adaptor put_vector_rs_to_string started");

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
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor put_vector_rs_to_string completed");
    return result;
}

char* put_subscription_rs_to_string(PutSubscriptionRS* rs) {
    logWriter(LOG_DEBUG, "adaptor put_subscription_rs_to_string started");

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
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor put_subscription_rs_to_string completed");
    return result;
}

char* query_vector_rs_to_string(QueryVectorRS* rs) {
    logWriter(LOG_DEBUG, "adaptor query_vector_rs_to_string started");

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
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor query_vector_rs_to_string completed");
    return result;
}

char* query_vector_wrapper_rs_to_string(QueryVectorRSWrapper* rs) {
    logWriter(LOG_DEBUG, "adaptor query_vector_wrapper_rs_to_string started");

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

    if (result == NULL) {
        logWriter(LOG_ERROR, "Memory allocation failed for result");
    }

    logWriter(LOG_DEBUG, "adaptor query_vector_wrapper_rs_to_string completed");
    return result;
}

Response add_db(char* db) {
    logWriter(LOG_DEBUG, "adaptor add_db started");

    char* dbFP = db_full_path(db);
    Response rs = newDBSL(dbFP);
    free(dbFP);

    logWriter(LOG_DEBUG, "adaptor add_db completed");
    return rs;
}

Response add_collection(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor add_collection started");

    char* collectionFP = collection_full_path(db, collection);
    Response rs = newCollectionSL(collectionFP);
    free(collectionFP);

    logWriter(LOG_DEBUG, "adaptor add_collection completed");
    return rs;
}

Response delete_collection(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor delete_collection started");

    char* collectionFP = collection_full_path(db, collection);
    Response rs = deleteCollectionSL(collectionFP);
    free(collectionFP);

    logWriter(LOG_DEBUG, "adaptor delete_collection completed");
    return rs;
}

CountRS count_collection(char* db) {
    logWriter(LOG_DEBUG, "adaptor count_collection started");

    char* collectionsBP = collection_base_path(db);
    CountRS rs = collectionCountSL(collectionsBP);
    free(collectionsBP);

    logWriter(LOG_DEBUG, "adaptor count_collection completed");
    return rs;
}



CollectionListRS list_collection(char* db) {
    logWriter(LOG_DEBUG, "adaptor list_collection started");

    char* collectionsBP = collection_base_path(db);
    CollectionListRS rs = collectionListSL(collectionsBP);
    free(collectionsBP);

    logWriter(LOG_DEBUG, "adaptor list_collection completed");
    return rs;
}

Response delete_vector(char* db, char* collection, char* hash) {
    logWriter(LOG_DEBUG, "adaptor delete_vector started");

    char* vectorBP = vector_base_path(db, collection);
    Response rs = deleteVectorSL(vectorBP, hash);
    free(vectorBP);

    logWriter(LOG_DEBUG, "adaptor delete_vector completed");
    return rs;
}

CountRS count_vector(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor count_vector started");

    char* vectorBP = vector_base_path(db, collection);
    CountRS rs = vectorCountSL(vectorBP);
    free(vectorBP);

    logWriter(LOG_DEBUG, "adaptor count_vector completed");
    return rs;
}

CountRS count_subscription(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor count_subscription started");

    char* subscriptionBP = subscription_base_path(db, collection);
    CountRS rs = subscriptionCountSL(subscriptionBP);
    free(subscriptionBP);

    logWriter(LOG_DEBUG, "adaptor count_subscription completed");
    return rs;
}

VectorListRS list_vector(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor list_vector started");

    char* vectorBP = vector_base_path(db, collection);
    VectorListRS rs = vectorListSL(vectorBP);
    free(vectorBP);

    logWriter(LOG_DEBUG, "adaptor list_vector completed");
    return rs;
}

SubscriptionListRS list_subscription(char* db, char* collection) {
    logWriter(LOG_DEBUG, "adaptor list_subscription started");

    char* subscriptionBP = subscription_base_path(db, collection);
    SubscriptionListRS rs = subscriptionListSL(subscriptionBP);
    free(subscriptionBP);

    logWriter(LOG_DEBUG, "adaptor list_subscription completed");
    return rs;
}

GetVectorRS get_vector(char* db, char* collection, char* hash) {
    logWriter(LOG_DEBUG, "adaptor get_vector started");

    char* vectorBP = vector_base_path(db, collection);
    GetVectorRS rs = getVectorSL(vectorBP, hash);
    free(vectorBP);

    logWriter(LOG_DEBUG, "adaptor get_vector completed");
    return rs;
}

GetSubscriptionRS get_subscription(char* db, char* collection, char* hash) {
    logWriter(LOG_DEBUG, "adaptor get_subscription started");

    char* subscriptionBP = subscription_base_path(db, collection);
    GetSubscriptionRS rs = getSubscriptionSL(subscriptionBP, hash);
    free(subscriptionBP);

    logWriter(LOG_DEBUG, "adaptor get_subscription completed");
    return rs;
}

PutVectorRS add_vector(char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite) {
    logWriter(LOG_DEBUG, "adaptor add_vector started");

    char* vectorBP = vector_base_path(db, collection);
    PutVectorRS rs = putVectorSL(vectorBP, ai_model, hash, vdim, vp, is_normal, overwrite);
    free(vectorBP);

    logWriter(LOG_DEBUG, "adaptor add_vector completed");
    return rs;
}

PutSubscriptionRS add_subscription(char* client_id, char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite, SubscriptionQueryOptions queryOptions) {
    logWriter(LOG_DEBUG, "adaptor add_subscription started");

    char* subscriptionBP = subscription_base_path(db, collection);
    PutSubscriptionRS rs = subscribeSL(client_id, subscriptionBP, ai_model, hash, vdim, vp, is_normal, overwrite, queryOptions);
    free(subscriptionBP);

    logWriter(LOG_DEBUG, "adaptor add_subscription completed");
    return rs;
}

QueryVectorRSWrapper query_vector(char* db, char* collection, char* ai_model, int vdim, double* vp, QueryOptions queryOptions) {
    logWriter(LOG_DEBUG, "adaptor query_vector started");

    char* vectorBP = vector_base_path(db, collection);
    QueryVectorRSWrapper rs = queryVectorSL(vectorBP, ai_model, vdim, vp, queryOptions);
    free(vectorBP);

    logWriter(LOG_DEBUG, "adaptor query_vector completed");
    return rs;
}

Response add_user(char* userName, char* password) {
    logWriter(LOG_DEBUG, "adaptor add_user started");
    User* user = (User*)malloc(sizeof(User));

    char* uuid = getUUID();
    strcpy(user->name, userName);
    strcpy(user->password, sha256(password));
    strcpy(user->uuid, uuid);
    user->userAccess = USER_ACCESS_NO_ACCESS;
    user->dbAccess = USER_ACCESS_FULL_ACCESS;
    user->collectionAccess = USER_ACCESS_FULL_ACCESS;
    user->vectorAccess = USER_ACCESS_FULL_ACCESS;
    
    Response rs = addUser(user);
    
    free(userName);
    free(password);
    free(uuid);
    logWriter(LOG_DEBUG, "adaptor add_user completed");

    return rs;
}

bool verifyAccess(char* op, char* obj, ClientInfo ClientInfo) {
    int access = true;

    if(strcmp(op,"add") == 0 && strcmp(obj, "db") == 0) {
        access = ClientInfo.dbAccess & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"add") == 0 && strcmp(obj, "collection") == 0) {
        access = ClientInfo.collectionAccess & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"count") == 0 && strcmp(obj, "collection") == 0) {
        access = ClientInfo.collectionAccess & USER_ACCESS_COUNT_ACCESS;
    } else if(strcmp(op,"list") == 0 && strcmp(obj, "collection") == 0) {
        access = ClientInfo.collectionAccess & USER_ACCESS_LIST_ACCESS;
    } else if(strcmp(op,"delete") == 0 && strcmp(obj, "collection") == 0) {
        access = ClientInfo.collectionAccess & USER_ACCESS_DELETE_ACCESS;
    } else if(strcmp(op,"put") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.vectorAccess & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"get") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.vectorAccess & USER_ACCESS_READ_ACCESS;
    } else if(strcmp(op,"delete") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.vectorAccess & USER_ACCESS_DELETE_ACCESS;
    } else if(strcmp(op,"count") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.vectorAccess & USER_ACCESS_COUNT_ACCESS;
    } else if(strcmp(op,"list") == 0 && strcmp(obj, "vector") == 0) {
        access = ClientInfo.vectorAccess & USER_ACCESS_LIST_ACCESS;
    } else if(strcmp(op,"query") == 0 && strcmp(obj, "vector") == 0) {
        printf("Inside Query");
        access = ClientInfo.vectorAccess & USER_ACCESS_READ_MULTIPLE_ACCESS;
    } else if(strcmp(op,"add") == 0 && strcmp(obj, "user") == 0) {
        access = ClientInfo.userAccess & USER_ACCESS_WRITE_ACCESS;
    } else if(strcmp(op,"add") == 0 && strcmp(obj, "subscription") == 0) {
        access = true;
    } else if(strcmp(op,"get") == 0 && strcmp(obj, "subscription") == 0) {
        access = true;
    } else if(strcmp(op,"list") == 0 && strcmp(obj, "subscription") == 0) {
        access = true;
    } else if(strcmp(op,"count") == 0 && strcmp(obj, "subscription") == 0) {
        access = true;
    } else if(strcmp(op,"delete") == 0 && strcmp(obj, "subscription") == 0) {
        access = true;
    }

    free(op);
    free(obj);
    return access;
}

char* do_db_ops(char* threadUUID, char* payload, ClientInfo clientInfo) {    
    logWriter(LOG_DEBUG, "adaptor do_db_ops started");

    char* result;

    logWriter(LOG_DEBUG, "adaptor metadataSB while db operations");
    StringBuilder metadataSB;
    initStringBuilder(&metadataSB, 10);

    logWriter(LOG_DEBUG, "adaptor errorSB while db operations");
    StringBuilder errorSB;
    initStringBuilder(&errorSB, 10);

    logWriter(LOG_DEBUG, "adaptor resultSB while db operations");
    StringBuilder resultSB;
    initStringBuilder(&resultSB, 10);

    logWriter(LOG_DEBUG, "adaptor clientResponseSB while db operations");
    StringBuilder clientResponseSB;
    initStringBuilder(&clientResponseSB, 10);

    appendToStringBuilder(&metadataSB, "\"metadata\": [");
    appendToStringBuilder(&metadataSB, "{\"response_id\": \"");
    char* responseID = strdup(threadUUID);
    appendToStringBuilder(&metadataSB, responseID);
    free(responseID);
    appendToStringBuilder(&metadataSB, "\"}");

    appendToStringBuilder(&errorSB, "\"error\": [");
    appendToStringBuilder(&resultSB, "\"result\": [");
    appendToStringBuilder(&clientResponseSB, "{");

    char* db = getDatabasePath();

    logWriter(LOG_DEBUG, "Payload: ");
    logWriter(LOG_DEBUG, payload);
    JsonNode* root = loadJson(payload);

    logWriter(LOG_INFO, "Check whether the DB path and Payload is valid");
    if(db == NULL) {
        logWriter(LOG_ERROR, "The base path provided for DB is NULL");
        appendToStringBuilder(&errorSB, "\"Invalid server state. Please restart the server with valid configuration.\"");     
    } else if(root == NULL) {
        logWriter(LOG_WARN, "The query provided is not a valid JSON"); 
        appendToStringBuilder(&errorSB, "\"Invalid Query (JSON format)\"");     
    } else {
            logWriter(LOG_INFO, "The provided DB path and Payload is valid");
            JsonNode* opNode =  searchJson(root, "op");
            JsonNode* objNode = searchJson(root, "obj");
            JsonNode* argsNode = searchJson(root, "args");
            
            char* op = (opNode != NULL) ? opNode->value : "empty";
            char* obj = (objNode != NULL) ? objNode->value : "empty";
            
            if(verifyAccess(strdup(op), strdup(obj), clientInfo)) {
                bool isError;
                if(strcmp(op, "add") == 0 && strcmp(obj, "user") == 0) {
                    logWriter(LOG_INFO, "Begin add user");

                    JsonNode* nameNode = searchJson(argsNode, "name");
                    if(nameNode == NULL || nameNode->value == NULL) {
                        logWriter(LOG_WARN, "Missing parameter: name or, name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: name\"");
                    } 

                    JsonNode* passwordNode = searchJson(argsNode, "password");
                    if(passwordNode == NULL || passwordNode->value == NULL) {
                        logWriter(LOG_WARN, "Missing parameter: password or, password provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: password\"");
                    } 

                    if(!isError) {
                        Response rs = add_user(nameNode->value, passwordNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "add") == 0 && strcmp(obj, "db") == 0) {
                    logWriter(LOG_INFO, "Begin add db");

                    JsonNode* dbNode = searchJson(argsNode, "db");
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    if(!isError) {
                        Response rs = add_db(dbNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "add") == 0 && strcmp(obj, "collection") == 0) {
                    logWriter(LOG_INFO, "Begin add collection");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        Response rs = add_collection(dbNode->value, collectionNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "delete") == 0 && strcmp(obj, "collection") == 0) {
                    logWriter(LOG_INFO, "Begin delete collection");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        Response rs = delete_collection(dbNode->value, collectionNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }
                    
                } else if(strcmp(op, "count") == 0 && strcmp(obj, "collection") == 0) {
                    logWriter(LOG_INFO, "Begin count collection");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    }

                    if(!isError) {
                        CountRS rs = count_collection(dbNode->value); 
                        char* result = count_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "list") == 0 && strcmp(obj, "collection") == 0) {
                    logWriter(LOG_INFO, "Begin list collection");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
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
                        
                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "delete") == 0 && strcmp(obj, "vector") == 0) {
                    logWriter(LOG_INFO, "Begin delete vector");
                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    JsonNode* hashNode = searchJson(argsNode, "hash");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(hashNode == NULL || hashNode->value == NULL || !isValidObjName(hashNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: vector (hash) or, vector (hash) provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: vector (hash), or vector (hash) provided is invalid\"");
                    } 

                    if(!isError) {
                        Response rs = delete_vector(dbNode->value, collectionNode->value, hashNode->value); 
                        char* result = response_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "count") == 0 && strcmp(obj, "vector") == 0) {
                    logWriter(LOG_INFO, "Begin count vector");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        CountRS rs = count_vector(dbNode->value, collectionNode->value); 
                        char* result = count_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else if(strcmp(op, "list") == 0 && strcmp(obj, "vector") == 0) {
                    logWriter(LOG_INFO, "Begin list vector");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
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

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }                
                    }

                } else if(strcmp(op, "get") == 0 && strcmp(obj, "vector") == 0) {
                    logWriter(LOG_INFO, "Begin get vector");
                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    JsonNode* hashNode = searchJson(argsNode, "hash");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(hashNode == NULL || hashNode->value == NULL || !isValidObjName(hashNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: vector (hash) or, vector (hash) provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: vector (hash), or vector (hash) provided is invalid\"");
                    } 
                    if(!isError) {
                        GetVectorRS rs = get_vector(dbNode->value, collectionNode->value, hashNode->value); 
                        char* result = vector_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }          
                    }

                } else if(strcmp(op, "put") == 0 && strcmp(obj, "vector") == 0) {
                    logWriter(LOG_INFO, "Begin put vector");

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    JsonNode* aiModelNode = searchJson(argsNode, "ai_model");
                    JsonNode* vdimNode = searchJson(argsNode, "vdim");
                    JsonNode* vpNode = searchJson(argsNode, "vp");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(aiModelNode == NULL || aiModelNode->value == NULL) {
                        logWriter(LOG_WARN, "Missing parameter: ai_model");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: ai model (ai_model)\"");
                    } 

                    int vdim = 0;

                    if(vdimNode == NULL || vdimNode->value == NULL || !isValidInteger(vdimNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: vector dimension (vdim)");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: vector dimension (vdim)\"");
                    } else {
                        vdim = atoi(vdimNode->value);
                    }

                    double vp[vdim];
                    if(vpNode == NULL) {
                        logWriter(LOG_WARN, "Missing parameter: vector points (vp)");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: vector points (vp)\"");
                    } else {
                        int i=0;
                        while(i<vdim && vpNode->children[i] != NULL) {
                            char* errptr;
                            vp[i] = strtod(vpNode->children[i]->value, &errptr);
                            
                            if (*errptr != '\0') {
                                logWriter(LOG_WARN, "Invalid vector points: ");
                                logWriter(LOG_WARN, errptr);
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

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added new node to subscribe queue");
                            enqueueSubscribeTrigMessage(rs.hash, clientInfo.client_id);
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }  
                        free(rs.errMsg);
                        free(rs.hash);
                    }

                } else if(strcmp(op, "query") == 0 && strcmp(obj, "vector") == 0) {
                    logWriter(LOG_INFO, "Begin query vector");

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    JsonNode* aiModelNode = searchJson(argsNode, "ai_model");
                    JsonNode* vdimNode = searchJson(argsNode, "vdim");
                    JsonNode* vpNode = searchJson(argsNode, "vp");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 
                    
                    if(aiModelNode == NULL || aiModelNode->value == NULL) {
                        logWriter(LOG_WARN, "Missing parameter: ai_model");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: ai model (ai_model)\"");
                    } 
                    
                    int vdim = 0; 
                    if(vdimNode == NULL || vdimNode->value == NULL || !isValidInteger(vdimNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: vector dimension (vdim)");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: vector dimension (vdim)\"");
                    } else {
                        vdim = atoi(vdimNode->value);
                    }
                    
                    if(vpNode == NULL) {
                        logWriter(LOG_WARN, "Missing parameter: vector points (vp)");
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
                                logWriter(LOG_WARN, "Invalid vector points");
                                logWriter(LOG_WARN, errptr);
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

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        } 
                    }

                } else if(strcmp(op, "add") == 0 && strcmp(obj, "subscription") == 0) {
                    logWriter(LOG_INFO, "Begin add subscription");

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    JsonNode* aiModelNode = searchJson(argsNode, "ai_model");
                    JsonNode* vdimNode = searchJson(argsNode, "vdim");
                    JsonNode* vpNode = searchJson(argsNode, "vp");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(aiModelNode == NULL || aiModelNode->value == NULL) {
                        logWriter(LOG_WARN, "Missing parameter: ai_model");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: ai model (ai_model)\"");
                    } 

                    int vdim = 0;

                    if(vdimNode == NULL || vdimNode->value == NULL || !isValidInteger(vdimNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: vector dimension (vdim)");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: vector dimension (vdim)\"");
                    } else {
                        vdim = atoi(vdimNode->value);
                    }

                    double vp[vdim];
                    if(vpNode == NULL) {
                        logWriter(LOG_WARN, "Missing parameter: vector points (vp)");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: vector points (vp)\"");
                    } else {
                        int i=0;
                        while(i<vdim && vpNode->children[i] != NULL) {
                            char* errptr;
                            vp[i] = strtod(vpNode->children[i]->value, &errptr);
                            
                            if (*errptr != '\0') {
                                logWriter(LOG_WARN, "Invalid vector points: ");
                                logWriter(LOG_WARN, errptr);
                                (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                                appendToStringBuilder(&errorSB, "\"Invalid vector points\"");
                                break;
                            }
                            i++;
                        }
                    }

                    char* errptr;
                    if(!isError) {
                        JsonNode* isNormalNode = searchJson(argsNode, "is_normal");
                        JsonNode* overwriteNode = searchJson(argsNode, "overwrite");

                        bool isNormal = (isNormalNode != NULL && strcasecmp(isNormalNode->value, "true") == 0) ? true : false;
                        bool overwrite = (overwriteNode != NULL && strcasecmp(overwriteNode->value, "true") == 0) ? true : false;

                        char* hash = getUUID();

                        SubscriptionQueryOptions queryOptions;
                        JsonNode* queryOptionsNode = searchJson(argsNode, "qOps");
                        
                        JsonNode* vdMethodNode = searchJson(queryOptionsNode, "vd_method");
                        queryOptions.vector_distance_method = (vdMethodNode != NULL && !isValidInteger(vdMethodNode->value)) ? atoi(vdMethodNode->value): 0;
                                               
                        JsonNode* logicalOpNode = searchJson(queryOptionsNode, "logical_op");
                        queryOptions.query_logical_op = (logicalOpNode != NULL && !isValidInteger(vdMethodNode->value))? atoi(logicalOpNode->value): 0;
                        
                        JsonNode* queryValueNode = searchJson(queryOptionsNode, "k_value");
                        double query_value = (queryValueNode != NULL && !isValidInteger(vdMethodNode->value)) ? strtod(queryValueNode->value, &errptr): 0;
                        if (*errptr != '\0') {
                            query_value = 0;
                        }   
                        queryOptions.query_value = query_value;
                      
                        JsonNode* pValueNode = searchJson(queryOptionsNode, "p_value");
                        double p_value = pValueNode != NULL ? strtod(pValueNode->value, &errptr) : 0;
                        if (*errptr != '\0') {
                            p_value = 0;
                        }
                        queryOptions.p_value = p_value;

                        PutSubscriptionRS rs = add_subscription(clientInfo.client_id, dbNode->value, collectionNode->value, aiModelNode->value, hash, vdim, vp, isNormal, overwrite, queryOptions); 
                        free(hash);

                        char* result = put_subscription_rs_to_string(&rs);

                        if(result != NULL) {
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }  
                        free(rs.errMsg);
                    }

                } else if(strcmp(op, "get") == 0 && strcmp(obj, "subscription") == 0) {
                    logWriter(LOG_INFO, "Begin get subscription");
                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    JsonNode* hashNode = searchJson(argsNode, "hash");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 
                    
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(hashNode == NULL || hashNode->value == NULL || !isValidObjName(hashNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: vector (hash) or, vector (hash) provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: vector (hash), or vector (hash) provided is invalid\"");
                    } 
                    if(!isError) {
                        GetSubscriptionRS rs = get_subscription(dbNode->value, collectionNode->value, hashNode->value); 
                        char* result = subscription_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }          
                    }

                } else if(strcmp(op, "list") == 0 && strcmp(obj, "subscription") == 0) {
                    logWriter(LOG_INFO, "Begin list subscription");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        SubscriptionListRS rs = list_subscription(dbNode->value, collectionNode->value); 
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
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }                
                    }

                } else if(strcmp(op, "count") == 0 && strcmp(obj, "subscription") == 0) {
                    logWriter(LOG_INFO, "Begin count subscription");
                    JsonNode* dbNode = searchJson(argsNode, "db");
                    
                    if(dbNode == NULL || dbNode->value == NULL || !isValidObjName(dbNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: db or, db name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: db or, db name provided is invalid\"");
                    } 

                    JsonNode* collectionNode = searchJson(argsNode, "collection");
                    if(collectionNode == NULL || collectionNode->value == NULL || !isValidObjName(collectionNode->value)) {
                        logWriter(LOG_WARN, "Missing parameter: collection or, collection name provided is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Missing parameter: collection or, collection name provided is invalid\"");
                    } 

                    if(!isError) {
                        CountRS rs = count_subscription(dbNode->value, collectionNode->value); 
                        char* result = count_rs_to_string(&rs);
                        free(rs.errMsg);

                        if(result != NULL) {
                            logWriter(LOG_INFO, "Added result to resultSB");
                            appendToStringBuilder(&resultSB, result);
                            free(result);
                        } else {
                            isError = true;
                            logWriter(LOG_ERROR, "DB Operation is unsuccessful");
                            appendToStringBuilder(&errorSB, "\"Internal server error\"");
                        }
                    }

                } else {
                    if(strcmp(op, "empty") == 0) {
                        logWriter(LOG_WARN, "Operation was missing in the payload");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Invalid Operation\"");
                    } else  if(strcmp(obj, "empty") == 0) {
                        logWriter(LOG_WARN, "DB Object provided in the payload is invalid");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Operation on invalid Object\"");
                    } else {
                        logWriter(LOG_WARN, "Operation and DB object is missing in the payload");
                        (isError) ? appendToStringBuilder(&errorSB, ", ") : (isError = true);
                        appendToStringBuilder(&errorSB, "\"Invalid query\"");            
                    }
                }

            } else {
                logWriter(LOG_WARN, "Authorization failure");
                appendToStringBuilder(&errorSB, "\"Authorization failure\"");     
            }

            freeJson(root);
    }

    free(clientInfo.client_id);
    
    appendToStringBuilder(&metadataSB, "]");
    appendToStringBuilder(&clientResponseSB, metadataSB.data);
    freeStringBuilder(&metadataSB);
    logWriter(LOG_DEBUG, "Added metadata to client response");
    appendToStringBuilder(&clientResponseSB, ", ");
    
    appendToStringBuilder(&resultSB, "]");
    appendToStringBuilder(&clientResponseSB, resultSB.data);
    freeStringBuilder(&resultSB);
    logWriter(LOG_DEBUG, "Added db operation result to client response");
    appendToStringBuilder(&clientResponseSB, ", ");

    appendToStringBuilder(&errorSB, "]");
    appendToStringBuilder(&clientResponseSB, errorSB.data);
    freeStringBuilder(&errorSB);
    logWriter(LOG_DEBUG, "Added errors if any to client response");

    appendToStringBuilder(&clientResponseSB, "}");

    result = strdup(clientResponseSB.data);
    freeStringBuilder(&clientResponseSB);
    
    logWriter(LOG_DEBUG, "adaptor do_db_ops completed");
    return result;
}


