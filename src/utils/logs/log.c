#include "interface/log.h"
#include "../ds/interface/hashmap.h"

static FILE* logfile;
static int serverLogLevel;
static LogMessageQueue messageQueue;
static HashMap threadRegister;

char* getLogLevelString(int code) {
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

int getLogLevelCode(char* loglevelStr) {
    if(strcmp(loglevelStr, "DEBUG") == 0) {
        return LOG_DEBUG;
    } else if(strcmp(loglevelStr, "INFO") == 0) {
        return LOG_INFO;
    } else if(strcmp(loglevelStr, "WARN") == 0) {
        return LOG_WARN;
    } else if(strcmp(loglevelStr, "ERROR") == 0) {
        return LOG_ERROR;
    } else if(strcmp(loglevelStr, "CRTITICAL") == 0) {
        return LOG_CRITICAL;
    } else {
        return -1;
    }
}

void freeLogMessageNode(LogMessageNode* node) {
    node->message = NULL;
    free(node->threadUUID);
    free(node->message);
    free(node);
}

void enqueueLogMessage(const int log_level, const char* threadUUID, const char* message) {
    LogMessageNode* newNode = (LogMessageNode*)malloc(sizeof(LogMessageNode));
    if (newNode == NULL) {
        printf("Failed to allocate memory for a new message node \n");
        return;
    }

    newNode->log_level = log_level;
    newNode->threadUUID = strdup(threadUUID);
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

LogMessageNode dequeueLogMessage() {
    LogMessageNode messageNode;
    pthread_mutex_lock(&messageQueue.mutex);

    while (messageQueue.head == NULL) {
        pthread_cond_wait(&messageQueue.cond, &messageQueue.mutex);
    }

    LogMessageNode* head = messageQueue.head;
    messageNode.log_level = head->log_level;
    messageNode.message = strdup(head->message);
    messageNode.threadUUID = strdup(head->threadUUID);

    messageQueue.head = head->next;
    freeLogMessageNode(head);

    pthread_mutex_unlock(&messageQueue.mutex);

    return messageNode;
}

void* logReaderThreadFunction(void* arg) {
    while (1) {
        time_t currentTime;
        LogMessageNode messageNode = dequeueLogMessage();
        time(&currentTime);


        char* log_level_str = getLogLevelString(messageNode.log_level);
        if(log_level_str && messageNode.log_level >= serverLogLevel) {
            char timestamp[25];
            struct tm *timeInfo = localtime(&currentTime);
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeInfo);

            if(logfile != NULL && fprintf(logfile, "%s %s %s %s\n", timestamp, log_level_str, messageNode.threadUUID, messageNode.message) >= 0) {
                fflush(logfile);
            } else {
                printf("%s %s %s %s\n", timestamp, log_level_str, messageNode.threadUUID, messageNode.message);
            }

        }

    }

    return NULL;
}

void initLogUtil(char* log_level, const char* path) {
    printf("log initLogUtil started\n");

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

    serverLogLevel = getLogLevelCode(log_level);
    printf("Server Log Level: %d\n", serverLogLevel);
    
    initializeHashMap(&threadRegister);
    printf("Map for Thead ID Register created.\n");
    pthread_t reader;
    if (pthread_create(&reader, NULL, logReaderThreadFunction, NULL) != 0) {
        printf("Error creating thread for async log.\n");
    } else {
        printf("Log message reader Thread Started.\n");
    }

    printf("log initLogUtil completed\n");
}

void logWriter(const int log_level, const char* message) {
    pthread_t thread = pthread_self();
    char buffer[25]; 
    snprintf(buffer, sizeof(buffer), "%p", (void *) thread);

    char* threadUUID = get(&threadRegister, buffer);
    if(threadUUID == NULL) {
        threadUUID = buffer;
    }
    
    enqueueLogMessage(log_level, threadUUID, message);
}

void freeLogUtil() {
    cleanupHashMap(&threadRegister);

    LogMessageNode* current = messageQueue.head;
    while (current != NULL) {
        LogMessageNode* nextNode = current->next;
        free(current->threadUUID);
        free(current->message);
        free(current);
        current = nextNode;
    }
    messageQueue.head = NULL; 

    pthread_mutex_destroy(&messageQueue.mutex);
    pthread_cond_destroy(&messageQueue.cond);

    printf("Log util resources cleanup successfull.\n");

    if(fclose(logfile) != 0) {
        printf("Error closing Log File.\n");
    }
}

void setLogThreadRegisterUUID(char* threadID, char* UUID) {
    insert(&threadRegister, threadID, UUID);
}

void removeLogThreadRegisterUUID(char* threadID) {
    delete(&threadRegister, threadID);
}
