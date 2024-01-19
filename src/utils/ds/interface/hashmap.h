#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define HASHMAP_SIZE 1024

typedef struct {
    char* key;
    char* value;
} KeyValuePair;

typedef struct HashMapNode {
    KeyValuePair pair;
    struct HashMapNode* next;
} HashMapNode;

typedef struct {
    HashMapNode* buckets[HASHMAP_SIZE];
    pthread_mutex_t locks[HASHMAP_SIZE];
} HashMap;

void initializeHashMap(HashMap* map);
void insert(HashMap* map, const char* key, char* value);
char* get(HashMap* map, const char* key);
void delete(HashMap* map, const char* key);
void cleanupHashMap(HashMap* map);