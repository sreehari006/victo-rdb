#include "../../ds/datastructures.h"
#include "../../servers/auth/includes/user_ops_proto.h"

Response add_user_sl(User* user);
Response change_password_sl(char* uuid, char* user_name, char* current_pass, char* new_pass, bool self);