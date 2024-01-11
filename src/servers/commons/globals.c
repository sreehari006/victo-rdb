#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "interface/globals.h"

static WebsocketParams websocketParams;
static bool isInitialized = false;  

void setWebSocketParams(WebsocketParams params) {
    if (!isInitialized) {
        websocketParams.dbBasePath = strdup(params.dbBasePath);
        websocketParams.ipAddress = strdup(params.ipAddress);
        websocketParams.port = params.port;
        websocketParams.maxStackSize = params.maxStackSize;
        isInitialized = true;
    }  
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

void cleanupDatabasePath() {
    if (isInitialized) {
        free(websocketParams.dbBasePath);
        free(websocketParams.ipAddress);
        isInitialized = false;
    }
}