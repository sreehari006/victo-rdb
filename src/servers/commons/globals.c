#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./includes/globals_proto.h"
#include "../../utils/logs/includes/log_proto.h"

static WebsocketParams websocket_params;
static bool isInitialized = false;  

void set_websocket_params(WebsocketParams params) {
    vt__log_writer(LOG_DEBUG, "globals setWebSocketParams started");
    if (!isInitialized) {
        websocket_params.ip_address = strdup(params.ip_address);
        websocket_params.port = params.port;
        websocket_params.enable_auth = params.enable_auth;
        isInitialized = true;
    }
    vt__log_writer(LOG_DEBUG, "globals setWebSocketParams completed");  
}

bool is_auth_enabled() {
    vt__log_writer(LOG_DEBUG, "globals getEnableAuth started");
    vt__log_writer(LOG_DEBUG, "globals getEnableAuth completed");
    return websocket_params.enable_auth;
}

char* get_websock_init_IP() {
    vt__log_writer(LOG_DEBUG, "globals getWebsockInitIP started");
    vt__log_writer(LOG_DEBUG, "globals getWebsockInitIP completed");
    return websocket_params.ip_address;
}

int get_websock_init_port() {
    vt__log_writer(LOG_DEBUG, "globals getWebsockInitPort started");
    vt__log_writer(LOG_DEBUG, "globals getWebsockInitPort completed");
    return websocket_params.port;
}

void free_websocket_params() {
    vt__log_writer(LOG_DEBUG, "globals cleanupWebSocketParams started");

    if (isInitialized) {
        free(websocket_params.ip_address);
        isInitialized = false;
    }
    
    vt__log_writer(LOG_DEBUG, "globals cleanupWebSocketParams completed");
}