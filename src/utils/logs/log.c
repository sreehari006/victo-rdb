#include "interface/log.h"

static FILE* logfile;
static LogMessageQueue messageQueue; 

void freeLogMessageNode(LogMessageNode* node) {
    node->message = NULL;
    free(node->message);
    free(node);
}

void enqueueLogMessage(const char* message) {
    LogMessageNode* newNode = (LogMessageNode*)malloc(sizeof(LogMessageNode));
    if (newNode == NULL) {
        printf("Failed to allocate memory for a new message node \n");
        return;
    }

    newNode->message = strdup(message);
    newNode->next = NULL;
    
    pthread_mutex_lock(&messageQueue.mutex);

    if (messageQueue.head == NULL) {
        messageQueue.head = newNode;
    } else {
        LogMessageNode* current = messageQueue.head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }

    pthread_cond_signal(&messageQueue.cond);
    pthread_mutex_unlock(&messageQueue.mutex);
}

char* dequeueLogMessage() {
    pthread_mutex_lock(&messageQueue.mutex);

    while (messageQueue.head == NULL) {
        pthread_cond_wait(&messageQueue.cond, &messageQueue.mutex);
    }

    LogMessageNode* head = messageQueue.head;
    char* message = strdup(head->message);

    messageQueue.head = head->next;
    freeLogMessageNode(head);

    pthread_mutex_unlock(&messageQueue.mutex);

    return message;
}

void* logReaderThreadFunction(void* arg) {
    while (1) {
        char* message = dequeueLogMessage();

        if(logfile != NULL && fprintf(logfile, "%s\n", message) >= 0) {
            fflush(logfile);
        } else {
            printf("%s\n", message);
        }
        
        free(message);
    }

    return NULL;
}

void initLogUtil(const char* path) {
    printf("Inside Log Util Init Method.\n");

    char logFilePath[strlen(path) + strlen("/log.txt") + 1];
    strcpy(logFilePath, path);
    strcat(logFilePath, "/log.txt");

    messageQueue.head = NULL;
    pthread_mutex_init(&messageQueue.mutex, NULL);
    pthread_cond_init(&messageQueue.cond, NULL);

    printf("Writing logs to: %s\n", logFilePath);
    logfile = fopen(logFilePath, "a");
    if (logfile == NULL) {
        printf("Error opening log file for writing.\n");
    }

    pthread_t reader;
    if (pthread_create(&reader, NULL, logReaderThreadFunction, NULL) != 0) {
        printf("Error creating thread for async log.\n");
    } else {
        printf("Log message reader Thread Started.\n");
    }
}

void logWriter(const char* message) {
    enqueueLogMessage(message);
}

void freeLogUtil() {
    LogMessageNode* current = messageQueue.head;
    while (current != NULL) {
        LogMessageNode* nextNode = current->next;
        free(current->message);
        free(current);
        current = nextNode;
    }
    messageQueue.head = NULL; 

    pthread_mutex_destroy(&messageQueue.mutex);
    pthread_cond_destroy(&messageQueue.cond);

    logWriter("Log util resources cleanup successfull.");

    if(fclose(logfile) != 0) {
        printf("Error closing Log File.\n");
    }
}