#include <stdbool.h>

typedef struct {
    char* ip_address;
    int port;
    bool enable_auth;
} WebsocketParams;

void set_websocket_params(WebsocketParams params);
char* get_websock_init_IP();
int get_websock_init_port();
bool is_auth_enabled();
void free_websocket_params();