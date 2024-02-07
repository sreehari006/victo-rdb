#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "src/servers/commons/includes/globals_proto.h"
#include "src/servers/websock/includes/server_proto.h"
#include "src/registry/includes/db_config_sl_proto.h"
#include "src/servers/auth/includes/auth_proto.h"
#include "src/utils/logs/includes/log_proto.h"
#include "src/utils/strings/includes/string_builder_proto.h"

void sigint_handler(int sig) {
    vt__log_writer(LOG_INFO, "Server resource cleanup");
    stop_websock_server();
    sleep(3);
    free_websocket_params();
    free_auth_util();
    sleep(2);
    vt__free_log_util();
}

int main(int argc, char *argv[]) {
    char* log_level = "INFO";
    WebsocketParams params;
    int i=1;
    
    params.ip_address = strdup("127.0.0.1");
    params.port = 2018;
    params.enable_auth = false;
    
    char* victo_base_path;
    while(i<argc) {
        if(strncmp(argv[i], "-", 1) == 0) {
            if(strcmp(argv[i], "-d") == 0) {
                victo_base_path = strdup(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-i") == 0) {
                params.ip_address = strdup(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-p") == 0) {
                if(vt__is_valid_integer(argv[i+1])) {
                    params.port = atoi(argv[i+1]);
                }
                i++;
            } else if(strcmp(argv[i], "-l") == 0) {
                log_level = strdup(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-a") == 0) {
                params.enable_auth = (argv[i+1] != NULL && strcasecmp(argv[i+1], "true") == 0) ? true : false;
                i++;
            }
        }
        
        i++;
    }
    
    // signal(SIGINT, sigint_handler);

    if(victo_base_path) {
        init_victo_config_sl(victo_base_path);
        char* victo_base_path = get_victo_base_path_sl();
        char* db_base_path = get_db_base_path_sl();
        char* logs_base_path = get_logs_base_path_sl();
        char* auth_base_path = get_auth_base_path_sl();
    }

    /* if(params.dbServerPath && initDBConfigSL(params.dbServerPath)) {
        params.dbBasePath = getDBBasePathSL(params.dbServerPath);
        char* dbLogPath = getDBLogPathSL(params.dbServerPath);
        char* authPath = getDBAuthPathSL(params.dbServerPath);
        
        initLogUtil(log_level, dbLogPath);
        free(dbLogPath);

        sleep(2);
        char sysThreadID[25]; 
        snprintf(sysThreadID, sizeof(sysThreadID), "%p", (void *) pthread_self());
        setLogThreadRegisterUUID(sysThreadID, "base");

        printf("** Starting server running..... **\n");
        logWriter(LOG_INFO, "Staring Victo server instance");
        logWriter(LOG_INFO, "DB Server Path: ");
        logWriter(LOG_INFO, params.dbServerPath);
        logWriter(LOG_INFO, "DB Base Path: ");
        logWriter(LOG_INFO, params.dbBasePath);
        logWriter(LOG_INFO, "DB Log Path: ");
        logWriter(LOG_INFO, dbLogPath);

        initAuthUtil(authPath);
        free(authPath);

        setWebSocketParams(params);
        startWebSockServer();
    } else {
        logWriter(LOG_CRITICAL, "Invalid server configuration.");
    } */

    return 0;
}