typedef struct User {
    char name[16];
    char password[65];
    char uuid[37];
    int userAccess;
    int dbAccess;
    int collectionAccess;
    int vectorAccess;
} User;

void initAuthUtil(const char* path);
void freeAuthUtil();