#include "../../commons/datastructures.h"

Response add_new_db_sl(const char* location);
DBListRS db_list_sl();
Response add_new_collection_sl(const char* db, const char* collection);
Response delete_collection_sl(const char* db, const char* collection);
CountRS collection_count_sl(const char* db);
CollectionListRS collection_list_sl(const char* db);
PutVectorRS put_vector_sl(char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite);
GetVectorRS get_vector_sl(char* db, char* collection, char* hash);
QueryVectorRSWrapper query_vector_sl(char* db, char* collection, char* ai_model, int vdim, double* vp, QueryOptions queryOptions);
CountRS vector_count_sl(char* db, char* collection);
VectorListRS vector_list_sl(char* db, char* collection);
Response delete_vector_sl(char* db, char* collection, char* hash);
PutSubscriptionRS subscribe_sl(char* client_id, char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite, SubscriptionQueryOptions queryOptions);
GetSubscriptionRS get_subscription_sl(char* db, char* collection, char* hash);
CountRS subscription_count_sl(char* db, char* collection);
SubscriptionListRS subscription_list_sl(char* db, char* collection);
Response unsubscribe_sl(char* db, char* collection, char* hash);
SubscriptionListNode* query_subscription_sl(char* db, char* collection, char* v_hash);
