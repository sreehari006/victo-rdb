#include <stdbool.h>

#include "../../../ds/datastructures.h"
#include "../../../commons/constants.h"


PutVectorRS put_vector(char* db, char* collection, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite);
GetVectorRS get_vector(char* db, char* collection, char* hash);
QueryVectorRSWrapper query_vector(char* db, char* collection, char* ai_model, int vdim, double* vp, QueryOptions queryOptions);
CountRS vector_count(char* db, char* collection);
VectorListRS vector_list(char* db, char* collection);
Response delete_vector(char* db, char* collection, char* hash);