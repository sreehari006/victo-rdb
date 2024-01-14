#include "interface/log.h"

static FILE* logfile;
static LogMessageQueue messageQueue; 

void enqueueLogMessage(const char* message) {
    LogMessageNode* newNode = (LogMessageNode*)malloc(sizeof(LogMessageNode));
    if (newNode == NULL) {
        perror("Failed to allocate memory for a new message node");
        exit(EXIT_FAILURE);
    }

    newNode->message = strdup(message);
    newNode->next = NULL;
    
    printf("\n Current Node Message: %s", newNode->message);
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
    free(head->message);
    free(head);

    pthread_mutex_unlock(&messageQueue.mutex);

    return message;
}

void* logReaderThreadFunction(void* arg) {
    printf("Inside Thread Fucntion");
    while (1) {
        printf("Keep reading message");
        char* message = dequeueLogMessage();
        printf("\n\n Before Writing to log: %s \n\n", message);
        fprintf(logfile, "%s\n", message);
        fflush(logfile);
        free(message);
    }

    return NULL;
}

void initLogUtil(const char* path) {
    printf("## Inside Log Util Init Method ##\n");
    logfile = fopen(path, "a");
    if (logfile == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    printf("## Log File Opened ##\n");

    messageQueue.head = NULL;
    pthread_mutex_init(&messageQueue.mutex, NULL);
    pthread_cond_init(&messageQueue.cond, NULL);

    pthread_t reader;
    if (pthread_create(&reader, NULL, logReaderThreadFunction, NULL) != 0) {
        perror("Error creating reader thread");
        exit(EXIT_FAILURE);
    }
    printf("## Reader Thread Started ##\n");
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
    fclose(logfile);
}