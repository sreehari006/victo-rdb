#include "./includes/hashmap_proto.h"

void vt__initialize_hashmap(HashMap *map) {
    for (int i = 0; i < HASHMAP_SIZE; i++) {
        map->buckets[i] = NULL;
        pthread_mutex_init(&map->locks[i], NULL);
    }
}

unsigned int hash(const char *key) {
    unsigned int hashval = 0;
    while (*key != '\0') {
        hashval = *key + (hashval << 5) - hashval;
        key++;
    }
    return hashval % HASHMAP_SIZE;
}

void vt__insert_hashmap(HashMap *map, const char *key, void* value, size_t value_size) {
    unsigned int index = hash(key);
    pthread_mutex_lock(&map->locks[index]);

    HashMapNode *newNode = malloc(sizeof(HashMapNode));
    newNode->pair.key = strdup(key);

    newNode->pair.value = malloc(value_size);
    if (newNode->pair.value == NULL) {
        printf("Error: HashMap InsertHashMap Memory allocation failed ");
    } else {
        memcpy(newNode->pair.value, value, value_size);
        newNode->next = map->buckets[index];
        map->buckets[index] = newNode;        
    }

    pthread_mutex_unlock(&map->locks[index]);
}

void* vt__get_hashmap(HashMap *map, const char *key) {
    unsigned int index = hash(key);
    void* value = NULL;

    pthread_mutex_lock(&map->locks[index]);

    HashMapNode *current = map->buckets[index];
    while (current != NULL) {
        if (strcmp(current->pair.key, key) == 0) {
            value = current->pair.value;
            break;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&map->locks[index]);

    return value;
}

void vt__delete_hashmap(HashMap *map, const char *key, cleanup_val_func func) {
    unsigned int index = hash(key);

    pthread_mutex_lock(&map->locks[index]);

    HashMapNode *current = map->buckets[index];
    HashMapNode *prev = NULL;

    while (current != NULL && strcmp(current->pair.key, key) != 0) {
        prev = current;
        current = current->next;
    }

    if (current != NULL) {
        if (prev != NULL) {
            prev->next = current->next;
        } else {
            map->buckets[index] = current->next;
        }

        free(current->pair.key);
        func(current->pair.value);
        free(current);
    }

    pthread_mutex_unlock(&map->locks[index]);
}

void vt__cleanup_hashmap(HashMap *map, cleanup_val_func func) {
    for (int i = 0; i < HASHMAP_SIZE; i++) {
        HashMapNode *current = map->buckets[i];
        while (current != NULL) {
            HashMapNode *next = current->next;
            free(current->pair.key);
            func(current->pair.value);
            free(current);
            current = next;
        }
        pthread_mutex_destroy(&map->locks[i]);
    }
}