#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "src/service_locator.h"
#include "src/servers/websock/interface/server.h"
#include "src/servers/commons/interface/globals.h"

void sigint_handler(int sig) {
    printf("## Data cleanup ##\n");
    stopWebSockServer();
    void cleanupDatabasePath();
}

int main(int argc, char *argv[]) {
    if(argv[1] != NULL) {
        printf("## Program Started with argument %s ##\n", argv[1]);
        signal(SIGINT, sigint_handler);
        setDatabasePath(argv[1]);
        startWebSockServer();
        initServiceLocator();
    } else {
        printf("## Invalid server configuration. ##");
    }

    return 0;
}