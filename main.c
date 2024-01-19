#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "src/service_locator.h"
#include "src/servers/websock/interface/server.h"
#include "src/servers/commons/interface/globals.h"
#include "src/utils/strings/interface/string_builder.h"
#include "src/utils/logs/interface/log.h"

void sigint_handler(int sig) {
    printf("## Data cleanup ##\n");
    stopWebSockServer();
    cleanupWebSocketParams();
    freeLogUtil();
}

int main(int argc, char *argv[]) {
    char* log_level = "INFO";
    WebsocketParams params;
    int i=1;
    
    params.dbBasePath = NULL;
    params.ipAddress = strdup("127.0.0.1");
    params.port = 2018;
    
    while(i<argc) {
        if(strncmp(argv[i], "-", 1) == 0) {
            if(strcmp(argv[i], "-d") == 0) {
                params.dbServerPath = strdup(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-i") == 0) {
                params.ipAddress = strdup(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-p") == 0) {
                if(isValidInteger(argv[i+1])) {
                    params.port = atoi(argv[i+1]);
                }
                i++;
            } else if(strcmp(argv[i], "-l") == 0) {
                log_level = strdup(argv[i+1]);
                i++;
            }
        }
        
        i++;
    }
    
    signal(SIGINT, sigint_handler);

    if(params.dbServerPath && initDBConfigSL(params.dbServerPath)) {
        params.dbBasePath = getDBBasePathSL(params.dbServerPath);
        params.dbLogPath = getDBLogPathSL(params.dbServerPath);
        
        printf("Starting server\n");
        initLogUtil(log_level, params.dbLogPath);
        logWriter(LOG_INFO, "Staring Victo server instance");
        logWriter(LOG_INFO, "DB Server Path: ");
        logWriter(LOG_INFO, params.dbServerPath);
        logWriter(LOG_INFO, "DB Base Path: ");
        logWriter(LOG_INFO, params.dbBasePath);
        logWriter(LOG_INFO, "DB Log Path: ");
        logWriter(LOG_INFO, params.dbLogPath);

        setWebSocketParams(params);
        startWebSockServer();
    } else {
        logWriter(LOG_CRITICAL, "Invalid server configuration.");
    }

    return 0;
}