#include <stdio.h>
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
        websocketParams.enableAuth = params.enableAuth;
        isInitialized = true;
    }
    logWriter(LOG_DEBUG, "globals setWebSocketParams completed");  
}

char* getDatabasePath() {
    logWriter(LOG_DEBUG, "globals getDatabasePath started");
    logWriter(LOG_DEBUG, "globals getDatabasePath completed");
    return websocketParams.dbBasePath;
}

bool getEnableAuth() {
    logWriter(LOG_DEBUG, "globals getEnableAuth started");
    logWriter(LOG_DEBUG, "globals getEnableAuth completed");
    return websocketParams.enableAuth;
}

char* getWebsockInitIP() {
    logWriter(LOG_DEBUG, "globals getWebsockInitIP started");
    logWriter(LOG_DEBUG, "globals getWebsockInitIP completed");
    return websocketParams.ipAddress;
}

int getWebsockInitPort() {
    logWriter(LOG_DEBUG, "globals getWebsockInitPort started");
    logWriter(LOG_DEBUG, "globals getWebsockInitPort completed");
    return websocketParams.port;
}

void cleanupWebSocketParams() {
    logWriter(LOG_DEBUG, "globals cleanupWebSocketParams started");

    if (isInitialized) {
        free(websocketParams.dbServerPath);
        free(websocketParams.dbBasePath);
        free(websocketParams.ipAddress);
        isInitialized = false;
    }
    
    logWriter(LOG_DEBUG, "globals cleanupWebSocketParams completed");
}