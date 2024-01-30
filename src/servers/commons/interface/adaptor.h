#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ClientInfo {
    char* client_id;
    int userAccess;
    int dbAccess;
    int collectionAccess;
    int vectorAccess;
} ClientInfo;

typedef struct SubscribeReplyInfo {
    char* client_id;
    char* vector_hash;
} SubscribeReplyInfo;

char* do_db_ops(char* threadUUID, char* payload, ClientInfo clientInfo);
void initSubscribeTrigQueue();
SubscribeReplyInfo querySubscription();
void freeSubscribeTrigMessagQueue();