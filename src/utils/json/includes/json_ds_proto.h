#define MAX_CHILDREN        8192

typedef struct JsonNode {
    struct JsonNode* parent;
    struct JsonNode* children[MAX_CHILDREN];
    char* type;
    char* key;
    char* value;
    int childIndex;
} JsonNode;