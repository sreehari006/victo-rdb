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

typedef void (*cleanup_val_func)(void*);
void vt__initialize_hashmap(HashMap* map);
void vt__insert_hashmap(HashMap* map, const char* key, void* value, size_t value_size);
void* vt__get_hashmap(HashMap* map, const char* key);
void vt__delete_hashmap(HashMap* map, const char* key, cleanup_val_func func);
void vt__cleanup_hashmap(HashMap* map, cleanup_val_func func);