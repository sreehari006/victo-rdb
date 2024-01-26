#include <stdbool.h>

typedef struct {
    char* dbServerPath;
    char* dbBasePath;
    char* ipAddress;
    int port;
    int maxStackSize;
    bool enableAuth;
} WebsocketParams;

void setWebSocketParams(WebsocketParams params);
char* getDatabasePath();
char* getWebsockInitIP();
int getWebsockInitPort();
bool getEnableAuth();
void cleanupWebSocketParams();