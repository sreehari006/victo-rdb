#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include "./includes/log_proto.h"
#include "../ds/includes/hashmap_proto.h"
#include "../time/includes/timestamp_proto.h"

typedef struct LogMessageNode {
    int log_level;
    char* thread_uuid;
    char* message;
    char* time_stamp;
    struct LogMessageNode* next;
} LogMessageNode;

typedef struct {
    LogMessageNode* head;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} LogMessageQueue;

typedef struct LogInfoParams {
    pthread_t parent_id;
    int log_level;
    char* message;
} LogInfoParams;

static FILE* log_file;
static int server_log_level;
static LogMessageQueue message_queue;
static HashMap thread_register;
static bool terminate_threads = false;

char* get_log_level_string(int code) {
    switch (code) {
        case 0: return "DEBUG";
                break;
        case 1: return "INFO";
                break;
        case 2: return "WARN";
                break;
        case 3: return "ERROR";
                break;
        case 4: return "CRITICAL";
                break;   
        default: return NULL;                                                             
    }
}

int get_log_level_code(char* log_level_str) {
    if(strcmp(log_level_str, "DEBUG") == 0) {
        return LOG_DEBUG;
    } else if(strcmp(log_level_str, "INFO") == 0) {
        return LOG_INFO;
    } else if(strcmp(log_level_str, "WARN") == 0) {
        return LOG_WARN;
    } else if(strcmp(log_level_str, "ERROR") == 0) {
        return LOG_ERROR;
    } else if(strcmp(log_level_str, "CRTITICAL") == 0) {
        return LOG_CRITICAL;
    } else {
        return -1;
    }
}

void free_log_message_node(LogMessageNode* node) {
    node->message = NULL;
    free(node->thread_uuid);
    free(node->message);
    free(node->time_stamp);
    free(node);
}

void free_message_node_value(void* value) {
    free(value);
}

void enqueue_log_message(const int log_level, char* thread_uuid, char* message, char* time_stamp) {
    LogMessageNode* newNode = (LogMessageNode*)malloc(sizeof(LogMessageNode));
    if (newNode == NULL) {
        printf("XX Failed to allocate memory for a new message node XX \n");
        return;
    }

    newNode->log_level = log_level;
    newNode->thread_uuid = strdup(thread_uuid);
    newNode->message = strdup(message);
    newNode->time_stamp = strdup(time_stamp);
    newNode->next = NULL;

    pthread_mutex_lock(&message_queue.mutex);

    if (message_queue.head == NULL) {
        message_queue.head = newNode;
    } else {
        LogMessageNode* current = message_queue.head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }

    pthread_cond_signal(&message_queue.cond);
    pthread_mutex_unlock(&message_queue.mutex);
}

LogMessageNode dequeue_log_message() {
    LogMessageNode messageNode;
    pthread_mutex_lock(&message_queue.mutex);

    while (message_queue.head == NULL) {
        pthread_cond_wait(&message_queue.cond, &message_queue.mutex);
    }

    LogMessageNode* head = message_queue.head;
    messageNode.log_level = head->log_level;
    messageNode.message = strdup(head->message);
    messageNode.thread_uuid = strdup(head->thread_uuid);
    messageNode.time_stamp = strdup(head->time_stamp);

    message_queue.head = head->next;
    free_log_message_node(head);

    pthread_mutex_unlock(&message_queue.mutex);

    return messageNode;
}

void* log_reader_TF(void* arg) {
    while (1) {
        LogMessageNode messageNode = dequeue_log_message();
    
        char* log_level_str = get_log_level_string(messageNode.log_level);
        if(log_level_str && messageNode.log_level >= server_log_level) {
            
            if(log_file != NULL && fprintf(log_file, "%s %s %s %s\n", messageNode.time_stamp, log_level_str, messageNode.thread_uuid, messageNode.message) >= 0) {
                fflush(log_file);
            } else {
                printf("%s %s %s %s\n", messageNode.time_stamp, log_level_str, messageNode.thread_uuid, messageNode.message);
            }

        }

        if (terminate_threads && message_queue.head == NULL) {
            pthread_exit(NULL);
        }
    }

    return NULL;
}

void vt__init_log_util(char* log_level, const char* path) {
    char log_file_path[strlen(path) + strlen("/log.txt") + 1];
    strcpy(log_file_path, path);
    strcat(log_file_path, "/log.txt");
    
    message_queue.head = NULL;
    pthread_mutex_init(&message_queue.mutex, NULL);
    pthread_cond_init(&message_queue.cond, NULL);

    vt__initialize_hashmap(&thread_register);

    log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        printf("XX Error opening log file for writing. XX \n");
    }

    server_log_level = get_log_level_code(log_level);

    pthread_t reader;
    if (pthread_create(&reader, NULL, log_reader_TF, NULL) != 0) {
        pthread_join(reader, NULL);
        vt__log_writer(LOG_INFO, "Error creating thread for async log.\n");
    } else {
        vt__log_writer(LOG_INFO, "Log message reader Thread Started.\n");
    }

    vt__log_writer(LOG_INFO, "log initLogUtil completed.\n");
}

void* log_writerTF(void* arg) {
    LogInfoParams* params = (LogInfoParams*) arg;

    char buffer[25]; 
    snprintf(buffer, sizeof(buffer), "%p", (void *) params->parent_id);

    char* thread_uuid = (char*) vt__get_hashmap(&thread_register, buffer);
    if(thread_uuid == NULL) {
        thread_uuid = buffer;
    }

    char timestamp_string[30];  
    vt__timestamp_to_string(timestamp_string, sizeof(timestamp_string));

    enqueue_log_message(params->log_level, thread_uuid, params->message, timestamp_string);

    free(params->message);
    free(params);

    return NULL;
}

void vt__log_writer(const int log_level, const char* message) {
    if(log_level < server_log_level) {
        return;
    }

    pthread_t parent_id, writer;
    parent_id = pthread_self();

    LogInfoParams* params = malloc(sizeof(LogInfoParams));
    params->parent_id = parent_id;
    params->log_level = log_level; 
    params->message = strdup(message);

    if (pthread_create(&writer, NULL, log_writerTF, (void*) params) != 0) {
        printf("Error creating child thread during log writer\n");
    }
}


void vt__free_log_util() {
    terminate_threads = true;

    cleanup_val_func cleanup_func_ptr = (cleanup_val_func) free_message_node_value;
    vt__cleanup_hashmap(&thread_register, cleanup_func_ptr);

    LogMessageNode* current = message_queue.head;
    while (current != NULL) {
        LogMessageNode* nextNode = current->next;
        free(current->thread_uuid);
        free(current->message);
        free(current);
        current = nextNode;
    }
    message_queue.head = NULL; 

    pthread_mutex_destroy(&message_queue.mutex);
    pthread_cond_destroy(&message_queue.cond);

    printf("** Log util resources cleanup successfull. **\n");

    if(fclose(log_file) != 0) {
        printf("XX Error closing Log File. XX\n");
    }
}

void vt__set_thread_id_on_register(char* thread_id, char* uuid) {
    char* uuid_copy = strdup(uuid);
    vt__insert_hashmap(&thread_register, thread_id, uuid_copy, strlen(uuid_copy) + 1);
}

void vt__remove_thread_id_from_register(char* thread_id) {
    cleanup_val_func cleanup_func_ptr = (cleanup_val_func) free_message_node_value;
    vt__delete_hashmap(&thread_register, thread_id, cleanup_func_ptr);
}
