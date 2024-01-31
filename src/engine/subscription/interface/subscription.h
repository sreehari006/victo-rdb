#include <stdbool.h>

#include "../../../ds/datastructures.h"
#include "../../../commons/constants.h"


PutSubscriptionRS subscribe(char* client_id, char* filename, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, SubscriptionQueryOptions queryOptions);
GetSubscriptionRS getSubscription(char* filename);
CountRS subscriptionCount(char* location);
SubscriptionListRS subscriptionList(const char* location);
Response unsubscribe(char* filename);