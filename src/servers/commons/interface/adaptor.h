#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ClientInfo {
    char* client_id;
    int userAccess;
    int dbAccess;
    int collectionAccess;
    int vectorAccess;
} ClientInfo;

char* do_db_ops(char* threadUUID, char* payload, ClientInfo clientInfo);