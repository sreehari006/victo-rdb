#include "../../../commons/datastructures.h"

Response add_new_collection(const char* db, const char* collection);
Response delete_collection(const char* db, const char* collection);
CountRS collection_count(const char* db);
CollectionListRS collection_list(const char* db);