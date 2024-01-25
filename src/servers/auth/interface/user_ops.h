#include <stdbool.h>

typedef struct User {
    char name[16];
    char password[65];
    char uuid[37];
    int userAccess;
    int dbAccess;
    int collectionAccess;
    int vectorAccess;
} User;

void addUser(User* user);
void updateUser(char* userName, User* user);
User findUser(char* userName);
bool authenticate(User* user);