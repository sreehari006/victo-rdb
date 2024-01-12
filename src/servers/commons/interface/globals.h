typedef struct {
    char* dbBasePath;
    char* ipAddress;
    int port;
    int maxStackSize;
} WebsocketParams;

void setWebSocketParams(WebsocketParams websocketParams);
char* getDatabasePath();
char* getWebsockInitIP();
int getWebsockInitPort();
void cleanupDatabasePath();