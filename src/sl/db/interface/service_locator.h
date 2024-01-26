#include <stdbool.h>
#include "../../../ds/datastructures.h"

Response newDBSL(const char* location);
Response newCollectionSL(const char* location);
Response deleteCollectionSL(const char* location);
CountRS collectionCountSL(const char* location);
CollectionListRS collectionListSL(const char* location);
PutVectorRS putVectorSL(char* location, char* ai_model, char* hash, int vdim, double* vp, bool is_normal, bool overwrite);
GetVectorRS getVectorSL(char* location, char* hash);
QueryVectorRSWrapper queryVectorSL(char* location, char* ai_model, int vdim, double* vp, QueryOptions queryOptions);
CountRS vectorCountSL(char* location);
VectorListRS vectorListSL(const char* location);
Response deleteVectorSL(char* location, char* hash);

