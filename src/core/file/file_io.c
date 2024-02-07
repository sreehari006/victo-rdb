#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>
#include "../../commons/constants.h"
#include "../../utils/files/includes/file_utils_proto.h"

int get_directory_count(const char* dir_path) {
    int count=0;
    struct dirent *entry;
    DIR *dir = opendir(dir_path);

    if(dir == NULL) {
        return 0;
    }

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char targetPath[PATH_MAX];
        snprintf(targetPath, sizeof(targetPath), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if(lstat(targetPath, &st) == -1) {
            continue;
        }

        if(S_ISDIR(st.st_mode)) {
            count++;
        } 
    }

    closedir(dir);
    return count;
}

char* remove_file_extension(const char* file_name) {
    char* baseName = basename((char *) file_name);
    char* dotPosition = strrchr(baseName, '.');
    if(dotPosition != NULL) {
        *dotPosition = '\0';
    }
    return strdup(baseName);
}

char** list_directory(const char* dir_path) {
    char** directories = NULL;
    int count=0;
    struct dirent *entry;
    DIR *dir = opendir(dir_path);

    if(dir == NULL) {
        return 0;
    }

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char targetPath[PATH_MAX];
        snprintf(targetPath, sizeof(targetPath), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if(lstat(targetPath, &st) == -1) {
            continue;
        }

        if(S_ISDIR(st.st_mode)) {
            directories = (char**) realloc(directories, (count+1) * sizeof(char*));
            directories[count] = strdup(entry->d_name);
            count++;
        } 
    }
    closedir(dir);

    if(count > 0) {
        directories = (char**) realloc(directories, (count+1) * sizeof(char*));
        directories[count] = NULL;
    }
    return directories;
}

char** list_files(const char* dir_path, int option, const char* ext) {
    char** files = NULL;
    int count=0;
    struct dirent *entry;
    DIR *dir = opendir(dir_path);
    if(dir == NULL) {
        return files;
    }

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char targetPath[PATH_MAX];
        snprintf(targetPath, sizeof(targetPath), "%s/%s", dir_path, entry->d_name);
        
        bool isRegularFile = false;

        #ifdef DT_REG
            if(entry->d_type == DT_REG) {
                isRegularFile = true;
            } else {
                struct stat st;
                if(stat(targetPath, &st) == 0 && S_ISREG(st.st_mode)) {
                    isRegularFile = true;
                }
            }
        #else
                struct stat st;
                if(stat(targetPath, &st) == 0 && S_ISREG(st.st_mode)) {
                    isRegularFile = true;
                }
            }
        #endif

        if(isRegularFile) {
            if(strstr(entry->d_name, ext) != NULL) {
                char* fileName;
                switch(option) {
                    case FILE_NAME_WITH_EXTENSION: 
                            fileName = entry->d_name;
                            break;
                    case FILE_NAME_WITHOUT_EXTENSION:
                            fileName = remove_file_extension(entry->d_name);
                            break;
                    default:
                            fileName = targetPath;
                } 

                files = (char**) realloc(files, (count+1) * sizeof(char*));
                files[count] = strdup(fileName);
                count++;

                if(option == FILE_NAME_WITHOUT_EXTENSION) {
                    free(fileName);
                }
            }
            
        } 
    }
    closedir(dir);

    if(count > 0) {
        files = (char**) realloc(files, (count+1) * sizeof(char*));
        files[count] = NULL;
    }
    return files;
}

int get_victo_files_count(const char* dir_path, const char* ext) {
    int count=0;
    struct dirent *entry;
    DIR *dir = opendir(dir_path);
    if(dir == NULL) {
        return count;
    }

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char targetPath[PATH_MAX];
        snprintf(targetPath, sizeof(targetPath), "%s/%s", dir_path, entry->d_name);
        
        bool isRegularFile = false;

        #ifdef DT_REG
            if(entry->d_type == DT_REG) {
                isRegularFile = true;
            } else {
                struct stat st;
                if(stat(targetPath, &st) == 0 && S_ISREG(st.st_mode)) {
                    isRegularFile = true;
                }
            }
        #else
                struct stat st;
                if(stat(targetPath, &st) == 0 && S_ISREG(st.st_mode)) {
                    isRegularFile = true;
                }
            }
        #endif

        if(isRegularFile) {
            if(strstr(entry->d_name, ext) != NULL) {
                count++; 
            }
            
        } 
    }
    closedir(dir);

    return count;
}

int delete_victo_file(const char* file_path) {
    if(strstr(file_path, VICTO_FILE_EXT) != NULL) {
        if(remove(file_path) !=  0) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return -1;
    }
}

int delete_subscription(const char* file_path) {
    if(strstr(file_path, SUSCRIP_FILE_EXT) != NULL) {
        if(remove(file_path) !=  0) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return -1;
    }
}

int delete_victo_collection(const char* dir_path) {
    int errCode = 0;
    struct dirent *entry;
    DIR *dir = opendir(dir_path);

    if(dir == NULL) {
        return errCode | 4;
    }

    if(!vt__dir_exists(dir_path)) {
        return errCode | 8;
    }

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char targetPath[PATH_MAX];
        snprintf(targetPath, sizeof(targetPath), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if(lstat(targetPath, &st) == -1) {
            continue;
        }

        if(S_ISDIR(st.st_mode)) {
            errCode = errCode | 1;
            // delete_files_and_subdirectories(targetPath);
            // rmdir(targetPath);
        } else {
            if(delete_victo_file(targetPath) < 0) {
                errCode = errCode | 2;
            }
            
        }
    }
    closedir(dir);

    return errCode;
}