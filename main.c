#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "src/servers/commons/interface/globals.h"
#include "src/servers/websock/interface/server.h"
#include "src/sl/db/interface/db_config.h"
#include "src/servers/auth/interface/auth.h"
#include "src/utils/logs/interface/log.h"
#include "src/utils/strings/interface/string_builder.h"

void sigint_handler(int sig) {
    printf("## Server resource cleanup ##\n");
    stopWebSockServer();
    cleanupWebSocketParams();
    freeAuthUtil();
    freeLogUtil();
}

int main(int argc, char *argv[]) {
    char* log_level = "INFO";
    WebsocketParams params;
    int i=1;
    
    params.dbBasePath = NULL;
    params.ipAddress = strdup("127.0.0.1");
    params.port = 2018;
    params.enableAuth = true;
    
    
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
            } else if(strcmp(argv[i], "-a") == 0) {
                params.enableAuth = (argv[i+1] != NULL && strcasecmp(argv[i+1], "true") == 0) ? true : false;
                i++;
            }
        }
        
        i++;
    }
    
    signal(SIGINT, sigint_handler);

    if(params.dbServerPath && initDBConfigSL(params.dbServerPath)) {
        params.dbBasePath = getDBBasePathSL(params.dbServerPath);
        const char* dbLogPath = getDBLogPathSL(params.dbServerPath);
        const char* authPath = getDBAuthPathSL(params.dbServerPath);
        
        printf("Starting server\n");
        logWriter(LOG_INFO, "Staring Victo server instance");
        logWriter(LOG_INFO, "DB Server Path: ");
        logWriter(LOG_INFO, params.dbServerPath);
        logWriter(LOG_INFO, "DB Base Path: ");
        logWriter(LOG_INFO, params.dbBasePath);
        logWriter(LOG_INFO, "DB Log Path: ");
        logWriter(LOG_INFO, dbLogPath);

        initLogUtil(log_level, dbLogPath);
        initAuthUtil(authPath);

        setWebSocketParams(params);
        startWebSockServer();
    } else {
        logWriter(LOG_CRITICAL, "Invalid server configuration.");
    }

    return 0;
}