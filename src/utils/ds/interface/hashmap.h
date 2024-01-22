#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define HASHMAP_SIZE 1024

typedef struct {
    char* key;
    void* value;
} KeyValuePair;

typedef struct HashMapNode {
    KeyValuePair pair;
    struct HashMapNode* next;
} HashMapNode;

typedef struct {
    HashMapNode* buckets[HASHMAP_SIZE];
    pthread_mutex_t locks[HASHMAP_SIZE];
} HashMap;

typedef void (*cleanupValueFunc)(void*);

void initializeHashMap(HashMap* map);
void insertHashMap(HashMap* map, const char* key, void* value, size_t valueSize);
void* getHashMap(HashMap* map, const char* key);
void deleteHashMap(HashMap* map, const char* key, cleanupValueFunc func);
void cleanupHashMap(HashMap* map, cleanupValueFunc func);