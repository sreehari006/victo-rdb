#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define LOG_DEBUG                               0
#define LOG_INFO                                1
#define LOG_WARN                                2
#define LOG_ERROR                               3
#define LOG_CRITICAL                            4

typedef struct LogMessageNode {
    int log_level;
    char* message;
    struct LogMessageNode* next;
} LogMessageNode;

typedef struct {
    LogMessageNode* head;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} LogMessageQueue;

void initLogUtil(char* log_level, const char* logPath);
void logWriter(const int log_level, const char* message);
void freeLogUtil();
