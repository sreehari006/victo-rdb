#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "./includes/stack_proto.h"
#include "../logs/includes/log_proto.h"

typedef enum {
    TOKEN_NONE,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET
} TokenType;

JsonNode* create_json_node(JsonNode* parent, char* type, char* key, char* value) {
    JsonNode* newNode = (JsonNode*) malloc(sizeof(JsonNode));
    if (newNode == NULL) {
        vt__log_writer(LOG_ERROR, "json createJsonNode Memory allocation failed");
        return NULL;
    }

    newNode->parent = parent;
    newNode->type =  type;
    newNode->key = key;
    newNode->value = value;
    newNode->childIndex = 0;

    return newNode;
}

void add_json_child(JsonNode* parent, JsonNode* child) {
    if(parent != NULL && child != NULL) {
        int i = parent->childIndex;
        parent->children[i] = child;
        parent->children[i+1] = NULL;
        parent->childIndex = i + 1;
    } else {
        vt__log_writer(LOG_ERROR, "json addJsonChild either parent or child node is empty");
    }
}

const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_NONE: return "NONE";
        case TOKEN_STRING: return "STRING";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_NULL: return "NULL";
        case TOKEN_COLON: return "COLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        default: return "UNKNOWN";
    }
}


void nullify_char_array(char arr[]) {
    for (int i=0; i<1024; i++) {
        arr[i] = '\0';
    }
}

char prev_non_space_char(int index, char* json_str) {
    int i = index-1;
    while(i>0 && isspace(json_str[i])) {
        i--;
    }
    return json_str[i];
}

void vt__print_json(JsonNode* node) {
    int i=0;
    while(node->children[i]!=NULL) {
        if(strcmp(node->children[i]->type, "object") == 0 || strcmp(node->children[i]->type, "array") == 0) {
            printf("%s: %s \n", node->children[i]->key, node->children[i]->type);
            vt__print_json(node->children[i]);
        } else{
            printf("%s: %s \n", node->children[i]->key, node->children[i]->value);
        }
        i++;
    }
}

JsonNode* vt__search_json(JsonNode* node, char* key) {
    JsonNode* resultNode = NULL;

    if(node == NULL)
        return resultNode;

    int i=0;
    while(node->children[i]!=NULL) {
        
        if(strcmp(node->children[i]->key, key) == 0) {
            return node->children[i];
        }

        i++;
    }
    return resultNode;
}

void vt__free_json(JsonNode* node) {
    int i=0;
    while(node->children[i]!=NULL) {
        if(strcmp(node->children[i]->type, "object") == 0 || strcmp(node->children[i]->type, "array") == 0) {
            vt__free_json(node->children[i]);
        } else {
            free(node->children[i]);
        }
        i++;
    }   

    free(node);

}

JsonNode* vt__load_json(char* json_str) {
    vt__log_writer(LOG_DEBUG, "json loadJson started");

    JsonNode* root = NULL;
    bool is_string = false;
    char token_buffer[1024];
    int token_i = 0;
    int first_non_char_index=0;
    bool is_error = false;

    while(isspace(json_str[first_non_char_index])) {
        first_non_char_index++;
    }

    if(json_str[first_non_char_index] != '{' && json_str[first_non_char_index] != '[') {
        return root;
    }

    JsonNodeStack jsonNodeStack;
    vt__initialize_JNS(&jsonNodeStack);

    DynamicStringStack keyTracker;
    vt__initialize_DSS(&keyTracker);

    for(int i=first_non_char_index; json_str[i] != '\0'; i++) {
        char c = json_str[i];
        if(!isspace(c)) {
            if(c == '\\' && json_str[i+1] == '\"') {
                token_buffer[token_i] = c;
                token_buffer[token_i++] = '\"';
                i++;
                continue;
            }

            if(c=='\"') {
                if(!is_string && token_i > 0) {
                    is_error = true;
                    break;
                }
                is_string = !is_string;
                continue;
            } 

            if(is_string) {
                if(c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',') {
                    token_buffer[token_i] = c;
                    token_i++;
                    continue;
                }
            }

            switch (c) {
                case '{': 
                    if(vt__is_empty_JNS(&jsonNodeStack) == 1) {
                        JsonNode* thisNode = create_json_node(NULL, "object", NULL, NULL);
                        if(thisNode == NULL) {
                            is_error = true;
                            break;
                        }

                        int response = vt__push_JNS(&jsonNodeStack, thisNode);
                        if(response < 0) {
                            is_error = true;
                            break;
                        } 
                        root = thisNode;
                    } else {
                        JsonNode* topNode = vt__peek_JNS(&jsonNodeStack);
                        if(topNode == NULL) {
                            is_error = true;
                            break;                              
                        }
                        char* key = vt__pop_DSS(&keyTracker);
                        if(key == NULL) {
                            is_error = true;
                            break;                                 
                        }                        
                        JsonNode* thisNode = create_json_node(topNode, "object", key, NULL);
                        if(thisNode == NULL) {
                            is_error = true;
                            break;
                        }
                        add_json_child(topNode, thisNode);
                        int response = vt__push_JNS(&jsonNodeStack, thisNode);
                        if(response < 0) {
                            is_error = true;
                            break;
                        } 
                    }
                    break;
                case '}': {
                        char p = prev_non_space_char(i, json_str);
                        if(p != '}' && p != ']') {
                            JsonNode* topNode = vt__peek_JNS(&jsonNodeStack);
                            if(topNode == NULL) {
                                is_error = true;
                                break;                              
                            }
                            char* key = vt__pop_DSS(&keyTracker);
                            if(key == NULL) {
                                is_error = true;
                                break;                                 
                            }
                            char* value = strdup(token_buffer);
                            JsonNode* child = create_json_node(topNode, "primitive", key, value);
                            if(child == NULL) {
                                is_error = true;
                                break;
                            }
                            add_json_child(topNode, child);
                            token_i=0; 
                            nullify_char_array(token_buffer); 
                        }
                        JsonNode* node = vt__pop_JNS(&jsonNodeStack);
                        if(node == NULL) {
                            is_error = true;
                            break; 
                        }
                    }
                    break;
                case '[': 
                    if(vt__is_empty_JNS(&jsonNodeStack) == 1) {
                        JsonNode* thisNode = create_json_node(NULL, "array", NULL, NULL);
                        if(thisNode == NULL) {
                            is_error = true;
                            break; 
                        }
                        int response = vt__push_JNS(&jsonNodeStack, thisNode);
                        if(response < 0) {
                            is_error = true;
                            break;
                        } 
                        root = thisNode;
                    } else {
                        JsonNode* topNode = vt__peek_JNS(&jsonNodeStack);
                        if(topNode == NULL) {
                            is_error = true;
                            break;                              
                        }
                        char* key = vt__pop_DSS(&keyTracker);
                        if(key == NULL) {
                            is_error = true;
                            break;                                 
                        }       
                        JsonNode* thisNode = create_json_node(topNode, "array", key, NULL);
                        if(thisNode == NULL) {
                            is_error = true;
                            break; 
                        }
                        add_json_child(topNode, thisNode);
                        int response = vt__push_JNS(&jsonNodeStack, thisNode);
                        if(response < 0) {
                            is_error = true;
                            break;
                        }
                    }
                    int response = vt__push_DSS(&keyTracker, "0");
                    if(response < 0) {
                        is_error = true;
                    }
                    break;
                case ']': {
                        char p = prev_non_space_char(i, json_str);
                        if(p != '}' && p != ']'){
                            JsonNode* topNode = vt__peek_JNS(&jsonNodeStack);
                            if(topNode == NULL) {
                                is_error = true;
                                break;                              
                            }
                            char* key = vt__pop_DSS(&keyTracker);
                            if(key == NULL) {
                                is_error = true;
                                break;                                 
                            }       
                            char* value = strdup(token_buffer);
                            JsonNode* child = create_json_node(topNode, "primitive", key, value);
                            if(child == NULL) {
                                is_error = true;
                                break; 
                            }
                            add_json_child(topNode, child);
                            token_i=0; 
                            nullify_char_array(token_buffer); 
                        }
                        JsonNode* node = vt__pop_JNS(&jsonNodeStack);
                        if(node == NULL) {
                            is_error = true;
                        }
                    }
                    break;
                case ':': {
                        int response = vt__push_DSS(&keyTracker, token_buffer);
                        if(response < 0) {
                            is_error = true;
                            break;
                        }
                        token_i=0; 
                        nullify_char_array(token_buffer); 
                    }
                    break;
                case ',': {
                        char p = prev_non_space_char(i, json_str);
                        if(p != '}' && p != ']'){
                            JsonNode* topNode = vt__peek_JNS(&jsonNodeStack);
                            if(topNode == NULL) {
                                is_error = true;
                                break;                                 
                            }       

                            if(strcmp(topNode->type, "object") == 0) {
                                char* key = vt__pop_DSS(&keyTracker);
                                if(key == NULL) {
                                    is_error = true;
                                    break;                                 
                                }       
                                char* value = strdup(token_buffer);
                                JsonNode* child = create_json_node(topNode, "primitive", key, value);
                                if(child == NULL) {
                                    is_error = true;
                                    break; 
                                }
                                add_json_child(topNode, child);
                            }

                            
                            if(strcmp(topNode->type, "array") == 0) {
                                char* key = vt__pop_DSS(&keyTracker);
                                if(key == NULL) {
                                    is_error = true;
                                    break;                                 
                                }   
                                char* value = strdup(token_buffer);
                                JsonNode* child = create_json_node(topNode, "primitive", key, value);
                                if(child == NULL) {
                                    is_error = true;
                                    break; 
                                }
                                add_json_child(topNode, child);
                                
                                char nextKey[4];
                                sprintf(nextKey, "%d", atoi(key)+1);
                                int response = vt__push_DSS(&keyTracker, nextKey);
                                if(response < 0) {
                                    is_error = true;
                                    break;
                                }
                            }
                            
                            token_i=0; 
                            nullify_char_array(token_buffer); 
                        }
                    }
                    break;
                default:
                    token_buffer[token_i] = c;
                    token_i++;
            }
            if(is_error) {
                break;
            }
        } else{
            if(is_string) {
                token_buffer[token_i] = c;
                token_i++;
            }
        }
    }

    vt__free_DSS(&keyTracker);

    if(is_error) {
        return NULL;    
    }

    vt__log_writer(LOG_DEBUG, "json loadJson completed");
    return root;
}