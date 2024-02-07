#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../ds/datastructures.h"

#define MAX_ACCESS_INDEX_TYPES  25

typedef struct ClientInfo {
    char* client_id;
    int user_access[MAX_ACCESS_INDEX_TYPES];
} ClientInfo;

typedef struct SubscribeTrigMsgNode {
    char* db;
    char* collection;
    char* vectorHash;
    char* clientID;
    struct SubscribeTrigMsgNode* next;
} SubscribeTrigMsgNode;

typedef struct SubscribeReplyInfo {
    char* client_id;
    char* vector_hash;
} SubscribeReplyInfo;

char* do_db_ops(char* threadUUID, char* payload, ClientInfo clientInfo);
void init_subscribe_trig_queue();
SubscribeTrigMsgNode* dequeue_subscribe_trig_message();
SubscriptionListNode* query_subscription(SubscribeTrigMsgNode* subscribeTrigMsgNode);
void free_subscribe_trig_messag_queue();
char* subscription_message(char* vector_hash, char* query_hash);