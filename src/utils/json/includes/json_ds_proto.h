typedef struct JsonNode {
    struct JsonNode* parent;
    struct JsonNode* children[8192];
    char* type;
    char* key;
    char* value;
    int childIndex;
} JsonNode;