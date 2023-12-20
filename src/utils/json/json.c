#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "interface/stack.h"

JsonNode* createJsonNode(JsonNode* parent, char* type, char* key, char* value) {
    JsonNode* newNode = (JsonNode*) malloc(sizeof(JsonNode));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->parent = parent;
    newNode->type =  type;
    newNode->key = key;
    newNode->value = value;
    newNode->childIndex = 0;

    return newNode;
}

void addJsonChild(JsonNode* parent, JsonNode* child) {
    int i = parent->childIndex;
    parent->children[i] = child;
    parent->children[i+1] = NULL;
    parent->childIndex = i + 1;
}

const char *tokenTypeToString(TokenType type) {
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


void nullifyCharArray(char arr[]) {
    for (int i=0; i<1024; i++) {
        arr[i] = '\0';
    }
}

char prevNonSpaceChar(int index, char* jsonStr) {
    int i = index-1;
    while(i>0 && isspace(jsonStr[i])) {
        i--;
    }
    return jsonStr[i];
}

void printJson(JsonNode* node) {
    int i=0;
    while(node->children[i]!=NULL) {
        if(strcmp(node->children[i]->type, "object") == 0 || strcmp(node->children[i]->type, "array") == 0) {
            printf("%s: %s \n", node->children[i]->key, node->children[i]->type);
            printJson(node->children[i]);
        } else{
            printf("%s: %s \n", node->children[i]->key, node->children[i]->value);
        }
        i++;
    }
}

JsonNode* searchJson(JsonNode* node, char* key) {
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

void freeJson(JsonNode* node) {
    int i=0;
    while(node->children[i]!=NULL) {
        if(strcmp(node->children[i]->type, "object") == 0 || strcmp(node->children[i]->type, "array") == 0) {
            freeJson(node->children[i]);
        } else {
            free(node->children[i]);
        }
        i++;
    }
    free(node);
}

JsonNode* loadJson(char* jsonStr) {
    JsonNode* root = NULL;
    bool isString = false;
    char tokenBuffer[1024];
    int token_i = 0;
    int firstNonCharIndex=0;
    bool isError = false;

    while(isspace(jsonStr[firstNonCharIndex])) {
        firstNonCharIndex++;
    }

    if(jsonStr[firstNonCharIndex] != '{' && jsonStr[firstNonCharIndex] != '[') {
        return root;
    }

    JsonNodeStack jsonNodeStack;
    initializeJNS(&jsonNodeStack);

    DynamicStringStack keyTracker;
    initializeDSS(&keyTracker);

    for(int i=firstNonCharIndex; jsonStr[i] != '\0'; i++) {
        char c = jsonStr[i];
        if(!isspace(c)) {
            if(c == '\\' && jsonStr[i+1] == '\"') {
                tokenBuffer[token_i] = c;
                tokenBuffer[token_i++] = '\"';
                i++;
                continue;
            }

            if(c=='\"') {
                if(!isString && token_i > 0) {
                    isError = true;
                    break;
                }
                isString = !isString;
                continue;
            } 

            if(isString) {
                if(c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',') {
                    tokenBuffer[token_i] = c;
                    token_i++;
                    continue;
                }
            }

            switch (c) {
                case '{': 
                    if(isEmptyJNS(&jsonNodeStack) == 1) {
                        JsonNode* thisNode = createJsonNode(NULL, "object", NULL, NULL);
                        int response = pushJNS(&jsonNodeStack, thisNode);
                        if(response < 0) {
                            isError = true;
                            break;
                        } 
                        root = thisNode;
                    } else {
                        JsonNode* topNode = peekJNS(&jsonNodeStack);
                        if(topNode == NULL) {
                            isError = true;
                            break;                              
                        }
                        char* key = popDSS(&keyTracker);
                        if(key == NULL) {
                            isError = true;
                            break;                                 
                        }                        
                        JsonNode* thisNode = createJsonNode(topNode, "object", key, NULL);
                        addJsonChild(topNode, thisNode);
                        int response = pushJNS(&jsonNodeStack, thisNode);
                        if(response < 0) {
                            isError = true;
                            break;
                        } 
                    }
                    break;
                case '}': {
                        char p = prevNonSpaceChar(i, jsonStr);
                        if(p != '}' && p != ']') {
                            JsonNode* topNode = peekJNS(&jsonNodeStack);
                            if(topNode == NULL) {
                                isError = true;
                                break;                              
                            }
                            char* key = popDSS(&keyTracker);
                            if(key == NULL) {
                                isError = true;
                                break;                                 
                            }
                            char* value = strdup(tokenBuffer);
                            JsonNode* child = createJsonNode(topNode, "primitive", key, value);
                            addJsonChild(topNode, child);
                            token_i=0; 
                            nullifyCharArray(tokenBuffer); 
                        }
                        JsonNode* node = popJNS(&jsonNodeStack);
                        if(node == NULL) {
                            isError = true;
                            break; 
                        }
                    }
                    break;
                case '[': 
                    if(isEmptyJNS(&jsonNodeStack) == 1) {
                        JsonNode* thisNode = createJsonNode(NULL, "array", NULL, NULL);
                        int response = pushJNS(&jsonNodeStack, thisNode);
                        if(response < 0) {
                            isError = true;
                            break;
                        } 
                        root = thisNode;
                    } else {
                        JsonNode* topNode = peekJNS(&jsonNodeStack);
                        if(topNode == NULL) {
                            isError = true;
                            break;                              
                        }
                        char* key = popDSS(&keyTracker);
                        if(key == NULL) {
                            isError = true;
                            break;                                 
                        }       
                        JsonNode* thisNode = createJsonNode(topNode, "array", key, NULL);
                        addJsonChild(topNode, thisNode);
                        int response = pushJNS(&jsonNodeStack, thisNode);
                        if(response < 0) {
                            isError = true;
                            break;
                        }
                    }
                    int response = pushDSS(&keyTracker, "0");
                    if(response < 0) {
                        isError = true;
                    }
                    break;
                case ']': {
                        char p = prevNonSpaceChar(i, jsonStr);
                        if(p != '}' && p != ']'){
                            JsonNode* topNode = peekJNS(&jsonNodeStack);
                            if(topNode == NULL) {
                                isError = true;
                                break;                              
                            }
                            char* key = popDSS(&keyTracker);
                            if(key == NULL) {
                                isError = true;
                                break;                                 
                            }       
                            char* value = strdup(tokenBuffer);
                            JsonNode* child = createJsonNode(topNode, "primitive", key, value);
                            addJsonChild(topNode, child);
                            token_i=0; 
                            nullifyCharArray(tokenBuffer); 
                        }
                        JsonNode* node = popJNS(&jsonNodeStack);
                        if(node == NULL) {
                            isError = true;
                        }
                    }
                    break;
                case ':': {
                        int response = pushDSS(&keyTracker, tokenBuffer);
                        if(response < 0) {
                            isError = true;
                            break;
                        }
                        token_i=0; 
                        nullifyCharArray(tokenBuffer); 
                    }
                    break;
                case ',': {
                        char p = prevNonSpaceChar(i, jsonStr);
                        if(p != '}' && p != ']'){
                            JsonNode* topNode = peekJNS(&jsonNodeStack);
                            if(topNode == NULL) {
                                isError = true;
                                break;                                 
                            }       

                            if(strcmp(topNode->type, "object") == 0) {
                                char* key = popDSS(&keyTracker);
                                if(key == NULL) {
                                    isError = true;
                                    break;                                 
                                }       
                                char* value = strdup(tokenBuffer);
                                JsonNode* child = createJsonNode(topNode, "primitive", key, value);
                                addJsonChild(topNode, child);
                            }

                            
                            if(strcmp(topNode->type, "array") == 0) {
                                char* key = popDSS(&keyTracker);
                                if(key == NULL) {
                                    isError = true;
                                    break;                                 
                                }   
                                char* value = strdup(tokenBuffer);
                                JsonNode* child = createJsonNode(topNode, "primitive", key, value);
                                addJsonChild(topNode, child);
                                
                                char nextKey[4];
                                sprintf(nextKey, "%d", atoi(key)+1);
                                int response = pushDSS(&keyTracker, nextKey);
                                if(response < 0) {
                                    isError = true;
                                    break;
                                }
                            }
                            
                            token_i=0; 
                            nullifyCharArray(tokenBuffer); 
                        }
                    }
                    break;
                default:
                    tokenBuffer[token_i] = c;
                    token_i++;
            }
            if(isError) {
                break;
            }
        } else{
            if(isString) {
                tokenBuffer[token_i] = c;
                token_i++;
            }
        }
    }

    // printf("Key Tracker: %d", isEmptyDSS(&keyTracker));
    freeDSS(&keyTracker);

    // printf("Node Tracker: %d", isEmptyDSS(&jsonNodeStack));
    // freeJNS(&jsonNodeStack);
    // printJson(root);
    // freeJson(root);

    if(isError) {
        return NULL;    
    }

    return root;
}