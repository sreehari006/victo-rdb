#include "../../../ds/datastructures.h"

GetVectorRS getVector(char* filename);
QueryVectorRS convert_to_query_vector_from_query(Node node, char* ai_model, int vdim, double* vp, QueryOptions queryOptions);