#include "interface/log.h"

static FILE* logfile;
static int serverLogLevel;
static LogMessageQueue messageQueue; 

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
    free(node->message);
    free(node);
}

void enqueueLogMessage(const int log_level, const char* message) {
    LogMessageNode* newNode = (LogMessageNode*)malloc(sizeof(LogMessageNode));
    if (newNode == NULL) {
        printf("Failed to allocate memory for a new message node \n");
        return;
    }

    newNode->log_level = log_level;
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


            if(logfile != NULL && fprintf(logfile, "%s %s %s\n", timestamp, log_level_str, messageNode.message) >= 0) {
                fflush(logfile);
            } else {
                printf("%s %s %s\n", timestamp, log_level_str, messageNode.message);
            }
        }

    }

    return NULL;
}

void initLogUtil(char* log_level, const char* path) {
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

    serverLogLevel = getLogLevelCode(log_level);
    printf("Server Log Level: %d", serverLogLevel);
    pthread_t reader;
    if (pthread_create(&reader, NULL, logReaderThreadFunction, NULL) != 0) {
        printf("Error creating thread for async log.\n");
    } else {
        printf("Log message reader Thread Started.\n");
    }
}

void logWriter(const int log_level, const char* message) {
    enqueueLogMessage(log_level, message);
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

    printf("Log util resources cleanup successfull.");

    if(fclose(logfile) != 0) {
        printf("Error closing Log File.\n");
    }
}