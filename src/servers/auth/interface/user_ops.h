#include <stdbool.h>
#include "../../../ds/datastructures.h"

typedef struct User {
    char name[16];
    char password[65];
    char uuid[37];
    int userAccess;
    int dbAccess;
    int collectionAccess;
    int vectorAccess;
} User;

Response addUser(User* user);
Response updateUser(char* userName, User* user);
bool authenticate(User* user);
Response deleteUser(char* userName);