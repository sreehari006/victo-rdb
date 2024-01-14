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
    initLogUtil("/Users/sreehari/projects/c/data/victodb/logs/log.txt");
    WebsocketParams params;
    int i=1;
    
    params.dbBasePath = NULL;
    params.ipAddress = strdup("127.0.0.1");
    params.port = 2018;
    
    while(i<argc) {
        if(strncmp(argv[i], "-", 1) == 0) {
            if(strcmp(argv[i], "-d") == 0) {
                params.dbBasePath = strdup(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-i") == 0) {
                params.ipAddress = strdup(argv[i+1]);
                i++;
            } else if(strcmp(argv[i], "-p") == 0) {
                if(isValidInteger(argv[i+1])) {
                    params.port = atoi(argv[i+1]);
                }
                i++;
            } 
        }
        i++;
    }
    
    signal(SIGINT, sigint_handler);
    if(params.dbBasePath) {
        printf("## Program Started with argument %s ##\n", params.dbBasePath);
        logWriter("1. Program Started");
        logWriter("2. Program Started");
        logWriter("3. Program Started");
        logWriter("4. Program Started");

        initServiceLocator();
        setWebSocketParams(params);
        startWebSockServer();
    } else {
        printf("## Invalid server configuration. ##\n");
    }

    return 0;
}