#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "interface/globals.h"
#include "../../utils/logs/interface/log.h"

static WebsocketParams websocketParams;
static bool isInitialized = false;  

void setWebSocketParams(WebsocketParams params) {
    logWriter(LOG_DEBUG, "globals setWebSocketParams started");
    if (!isInitialized) {
        websocketParams.dbServerPath = strdup(params.dbServerPath);
        websocketParams.dbBasePath = strdup(params.dbBasePath);
        websocketParams.ipAddress = strdup(params.ipAddress);
        websocketParams.port = params.port;
        websocketParams.maxStackSize = params.maxStackSize;
        isInitialized = true;
    }
    logWriter(LOG_DEBUG, "globals setWebSocketParams completed");  
}

char* getDatabasePath() {
    return websocketParams.dbBasePath;
}

char* getWebsockInitIP() {
    return websocketParams.ipAddress;
}

int getWebsockInitPort() {
    return websocketParams.port;
}

void cleanupWebSocketParams() {
    if (isInitialized) {
        free(websocketParams.dbServerPath);
        free(websocketParams.dbBasePath);
        free(websocketParams.ipAddress);
        isInitialized = false;
    }
}