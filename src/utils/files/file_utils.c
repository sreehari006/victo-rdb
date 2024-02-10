#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

bool vt__file_exists(const char* filename) {
        return (access(filename, F_OK) != -1);
}


bool vt__dir_exists(const char* dir_path) {
        DIR* dir = opendir(dir_path);
        if(dir) {
            closedir(dir);
            return true;
        }
        return false;
}