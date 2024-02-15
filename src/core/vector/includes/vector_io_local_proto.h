#include "../../../commons/datastructures.h"

GetVectorRS get_vector_local(char* filename);
QueryVectorRS convert_to_query_vector_from_query(Node node, char* ai_model, int vdim, double* vp, QueryOptions queryOptions);
bool compare(int op, double v1, double v2) ;