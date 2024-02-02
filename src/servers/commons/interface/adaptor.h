#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACCESS_INDEX_TYPES  25

typedef struct ClientInfo {
    char* client_id;
    int user_access[MAX_ACCESS_INDEX_TYPES];
} ClientInfo;

typedef struct SubscribeReplyInfo {
    char* client_id;
    char* vector_hash;
} SubscribeReplyInfo;

char* do_db_ops(char* threadUUID, char* payload, ClientInfo clientInfo);
void initSubscribeTrigQueue();
SubscribeReplyInfo querySubscription();
void freeSubscribeTrigMessagQueue();