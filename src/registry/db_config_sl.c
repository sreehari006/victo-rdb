#include "../core/base/includes/db_config_proto.h"

void init_victo_config_sl(const char* location) {
    init_victo_config(location);
}

char* get_victo_base_path_sl() {
    return get_victo_base_path();
}

char* get_db_base_path_sl() {
    return get_db_base_path();
}

char* get_logs_base_path_sl() {
    return get_logs_base_path();
}

char* get_auth_base_path_sl() {
    return get_auth_base_path();
}

void free_victo_config_sl() {
    free_victo_config();
}