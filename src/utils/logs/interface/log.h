#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

typedef struct LogMessageNode {
    char* message;
    struct LogMessageNode* next;
} LogMessageNode;

typedef struct {
    LogMessageNode* head;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} LogMessageQueue;

void initLogUtil(const char* logPath);
void logWriter(const char* message);
void freeLogUtil();
