#include "../../../ds/datastructures.h"

PutSubscriptionRS subscribe(char* client_id, char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite, SubscriptionQueryOptions queryOptions);
GetSubscriptionRS get_subscription(char* db, char* collection, char* hash);
CountRS subscription_count(char* db, char* collection);
SubscriptionListRS subscription_list(char* db, char* collection);
Response unsubscribe(char* db, char* collection, char* hash);
SubscriptionListNode* query_subscription(char* db, char* collection, char* v_hash);