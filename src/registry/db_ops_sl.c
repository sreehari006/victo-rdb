#include <stdbool.h>
#include "../commons/datastructures.h"
#include "../core/db/includes/db_proto.h"
#include "../core/collection/includes/collection_proto.h"
#include "../core/vector/includes/vector_io_proto.h"
#include "../core/subscription/includes/subscription_proto.h"

Response add_new_db_sl(const char* location) {
    return add_new_db(location);
}

DBListRS db_list_sl() {
    return db_list();
}


Response add_new_collection_sl(const char* db, const char* collection) {
    return add_new_collection(db, collection);
}

Response delete_collection_sl(const char* db, const char* collection) {
    return delete_collection(db, collection);
}

CountRS collection_count_sl(const char* db) {
    return collection_count(db);
}

CollectionListRS collection_list_sl(const char* db) {
    return collection_list(db);
}

PutVectorRS put_vector_sl(char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite) {
    return put_vector(db, collection, ai_model, hash, vdim, vp, is_normal, overwrite);
}

GetVectorRS get_vector_sl(char* db, char* collection, char* hash) {
    return get_vector(db, collection, hash);
}

QueryVectorRSWrapper query_vector_sl(char* db, char* collection, char* ai_model, int vdim, double* vp, QueryOptions queryOptions) {
    return query_vector(db, collection, ai_model, vdim, vp, queryOptions);
}

CountRS vector_count_sl(char* db, char* collection) {
    return vector_count(db, collection);
}

VectorListRS vector_list_sl(char* db, char* collection) {
    return vector_list(db, collection);
}

Response delete_vector_sl(char* db, char* collection, char* hash) {
    return delete_vector(db, collection, hash);
}

PutSubscriptionRS subscribe_sl(char* client_id, char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite, SubscriptionQueryOptions queryOptions) {
    return subscribe(client_id, db, collection, ai_model, hash, vdim, vp, is_normal, overwrite, queryOptions);
}

GetSubscriptionRS get_subscription_sl(char* db, char* collection, char* hash) {
    return get_subscription(db, collection, hash);
}

CountRS subscription_count_sl(char* db, char* collection) {
    return subscription_count(db, collection);
}

SubscriptionListRS subscription_list_sl(char* db, char* collection) {
    return subscription_list(db, collection);
}

Response unsubscribe_sl(char* db, char* collection, char* hash) {
    return unsubscribe(db, collection, hash);
}

SubscriptionListNode* query_subscription_sl(char* db, char* collection, char* v_hash) {
    return query_subscription(db, collection, v_hash);
}