#include <stdbool.h>

#include "../../../ds/datastructures.h"
#include "../../../commons/constants.h"


PutVectorRS putVector(char* filename, char* ai_model, char* hash, int vdim, double* vp, bool is_normal);
GetVectorRS getVector(char* filename);
QueryVectorRSWrapper queryVector(char* location, char* ai_model, int vdim, double* vp, QueryOptions queryOptions);
CountRS vectorCount(char* location);
VectorListRS vectorList(const char* location);
Response deleteVector(char* filename);