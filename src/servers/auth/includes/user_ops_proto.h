#include <stdbool.h>
#include "../../../ds/datastructures.h"

#define USER_ALREADY_EXIST_CODE                 1
#define USER_ALREADY_EXIST_MSG                  "User already exist" 
#define USER_GUEST                              "GUEST"
#define USER_ACCESS_NO_ACCESS                   0
#define USER_ACCESS_FULL_ACCESS                 32767
#define USER_ACCESS_READ_ACCESS                 1
#define USER_ACCESS_READ_MULTIPLE_ACCESS        2
#define USER_ACCESS_WRITE_ACCESS                4
#define USER_ACCESS_UPDATE_ACCESS               8 
#define USER_ACCESS_DELETE_ACCESS               16
#define USER_ACCESS_COUNT_ACCESS                32
#define USER_ACCESS_LIST_ACCESS                 64   

#define USER_ACCESS_INDEX                       0
#define DB_ACCESS_INDEX                         1
#define COLLECTION_ACCESS_INDEX                 2
#define VECTOR_ACCESS_INDEX                     3
#define SUBSCRIPTION_ACCESS_INDEX               4

#define MAX_ACCESS_INDEX_TYPES                 25
typedef struct User {
    char name[17];
    char password[65];
    char uuid[37];
    int user_access[MAX_ACCESS_INDEX_TYPES];
    int dbAccess;
    int collectionAccess;
    int vectorAccess;
} User;

Response add_user(User* user);
Response update_user(char* user_name, User* user);
User* authenticate(char* user_name, char* password);
Response delete_user(char* user_name);