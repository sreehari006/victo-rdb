#include "./json_ds_proto.h"

JsonNode* vt__load_json(char* jsonStr);
JsonNode* vt__search_json(JsonNode* node, char* key);
void vt__print_json(JsonNode* node);
void vt__free_json(JsonNode* node);