#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <stdbool.h>

#define VP_DIMENSION_MAX            2048

typedef struct response {
    int errCode;
    char* errMsg;
} Response;

typedef struct countRS {
    int errCode;
    char* errMsg;
    int count;
} CountRS;

typedef struct dbListRS {
    int errCode;
    char* errMsg;
    char** dbs;
} DBListRS;

typedef struct collectionListRS {
    int errCode;
    char* errMsg;
    char** collections;
} CollectionListRS;

typedef struct vectorListRS {
    int errCode;
    char* errMsg;
    char** vectors;
} VectorListRS;

typedef struct node {
    char ai_model[64];
    char hash[64];
    char normal[2];
    int vdim;
    double vp[VP_DIMENSION_MAX];
} Node;

typedef struct putVectorRS {
    int errCode;
    char* errMsg;
    char* hash;
} PutVectorRS;

typedef struct getVectorRS {
    int errCode;
    char* errMsg;
    Node node;
} GetVectorRS;

typedef struct vectorDistanceRS {
    int errCode;
    char* errMsg;
    double distance;
} VectorDistanceRS;

typedef struct queryVectorRS {
    int errCode;
    char errMsg[18];
    char ai_model[64];
    char normal[2];
    char hash[64];
    int vdim;
    double distance;
} QueryVectorRS;

typedef struct queryVectorRSWrapper {
    int errCode;
    char* errMsg;
    int queryCount;
    int faultCount;
    QueryVectorRS* queryVectorRS;
    QueryVectorRS* faultVectorRS;
} QueryVectorRSWrapper;

typedef struct queryVectorLinkedList {
    QueryVectorRS queryVectorRS;
    struct queryVectorLinkedList *next;
} QueryVectorLinkedList;

typedef struct queryOptions {
    int vector_distance_method;
    int query_limit;
    int query_logical_op;
    double query_value;
    bool include_fault;
    double p_value;
    bool do_normal;
    bool order;
} QueryOptions;

typedef struct SubscriptionQueryOptions {
    int vector_distance_method;
    int query_logical_op;
    double query_value;
    double p_value;
} SubscriptionQueryOptions;

typedef struct SubscriptionNode {
    char client_id[37];
    char ai_model[64];
    char hash[64];
    char normal[2];
    int vdim;
    int vector_distance_method;
    int query_logical_op;
    double query_value;
    double p_value;
    double vp[VP_DIMENSION_MAX];
} SubscriptionNode;

typedef struct SubscriptionListRS {
    int errCode;
    char* errMsg;
    char** subscriptions;
} SubscriptionListRS;

typedef struct PutSubscriptionRS {
    int errCode;
    char* errMsg;
    char* hash;
} PutSubscriptionRS;

typedef struct GetSubscriptionRS {
    int errCode;
    char* errMsg;
    SubscriptionNode node;
} GetSubscriptionRS;

typedef struct SubscriptionMessageNode {
    char* vector_hash;
    char* query_hash;
    struct SubscriptionMessageNode *next;
} SubscriptionMessageNode;

typedef struct SubscriptionListNode {
    char* client_id;
    struct SubscriptionMessageNode *message;
    struct SubscriptionListNode *next;
} SubscriptionListNode;




#endif
