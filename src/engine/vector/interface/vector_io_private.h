#include <stdbool.h>
#include "../../../ds/datastructures.h"

PutVectorRS put_vector (char* filename, char* ai_model, char* hash, int vdim, double* vp, char* normal);
QueryVectorRS convert_to_query_vector_from_query(Node node, char* ai_model, int vdim, double* vp, QueryOptions queryOptions);
QueryVectorRS* convert_to_query_vector_from_list(QueryVectorLinkedList* listHead, int count, int limit);
void add_query_result_to_list(QueryVectorLinkedList** listHead, QueryVectorRS qvRS, bool isValid, bool order);
VectorDistanceRS vector_distance(int method, int vdim, double* vp1, double* vp2, QueryOptions queryOptions);
void free_linked_list(QueryVectorLinkedList* head);
bool compare(int op, double v1, double v2);