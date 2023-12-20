#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <shlwapi.h>
    #pragma comment(lib, "Shlwapi.lib")
#else
    #include <dirent.h>
    #include <limits.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <errno.h>
    #include <libgen.h>
#endif

#include "../../commons/constants.h"
#include "./interface/file_io.h"

__attribute__((visibility("hidden"))) 
bool fileExists(const char* filename) {
    #if defined(_WIN32) || defined(_WIN64)
        DWORD fileAttributes = getFileAttributes(filename);
        return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
    #else
        return (access(filename, F_OK) != -1);
    #endif
}

__attribute__((visibility("hidden"))) 
bool dirExists(const char* dirPath) {
    #if defined(_WIN32) || defined(_WIN64)
        DWORD fileAttributes = getFileAttributes(dirPath);
        return (fileAttributes != INVALID_FILE_ATTRIBUTES && fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    #else
        DIR* dir = opendir(dirPath);
        if(dir) {
            closedir(dir);
            return true;
        }
        return false;
    #endif
}

__attribute__((visibility("hidden"))) 
void delete_files_and_subdirectories(const char* dirPath) {
    #if defined(_WIN32) || defined(_WIN64)
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        char targetPath[MAX_PATH];

        snprintf(targetPath, sizeof(targetPath), "%s\\*", dirPath);
        hFind = FindFirstFile(targetPath, &findFileData);

        if(hFind == INVALID_HANDLE_VALUE) {
            return;
        }

        do {
            if(strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..")) {
                continue;
            }

            snprintf(targetPath, sizeof(targetPath), "%s\\%s", dirPath, findFileData.cFileName);

            if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                delete_files_and_subdirectories(targetPath);
                RemoveDirectory(targetPath);
            } else {
                DeleteFile(targetPath);
            }

        } while(FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
    #else
        struct dirent *entry;
        DIR *dir = opendir(dirPath);

        if(dir == NULL) {
            return;
        }

        while((entry = readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char targetPath[PATH_MAX];
            snprintf(targetPath, sizeof(targetPath), "%s/%s", dirPath, entry->d_name);

            struct stat st;
            if(lstat(targetPath, &st) == -1) {
                continue;
            }

            if(S_ISDIR(st.st_mode)) {
                delete_files_and_subdirectories(targetPath);
                rmdir(targetPath);
            } else {
                remove(targetPath);
            }
        }
        closedir(dir);
    #endif
}

__attribute__((visibility("hidden"))) 
int get_directory_count(const char* dirPath) {
    int count=0;
    #if defined(_WIN32) || defined(_WIN64)
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        char targetPath[MAX_PATH];

        snprintf(targetPath, sizeof(targetPath), "%s\\*", dirPath);
        hFind = FindFirstFile(targetPath, &findFileData);

        if(hFind == INVALID_HANDLE_VALUE) {
            return;
        }

        do {
            if(strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..")) {
                continue;
            }

            snprintf(targetPath, sizeof(targetPath), "%s\\%s", dirPath, findFileData.cFileName);

            if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                count++;
            } 

        } while(FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
    #else
        struct dirent *entry;
        DIR *dir = opendir(dirPath);

        if(dir == NULL) {
            return 0;
        }

        while((entry = readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char targetPath[PATH_MAX];
            snprintf(targetPath, sizeof(targetPath), "%s/%s", dirPath, entry->d_name);

            struct stat st;
            if(lstat(targetPath, &st) == -1) {
                continue;
            }

            if(S_ISDIR(st.st_mode)) {
                count++;
            } 
        }
        closedir(dir);
    #endif
    return count;
}

__attribute__((visibility("hidden"))) 
char** list_directory(const char* dirPath) {
    char** directories = NULL;
    int count=0;
    #if defined(_WIN32) || defined(_WIN64)
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        char targetPath[MAX_PATH];

        snprintf(targetPath, sizeof(targetPath), "%s\\*", dirPath);
        hFind = FindFirstFile(targetPath, &findFileData);

        if(hFind == INVALID_HANDLE_VALUE) {
            return;
        }

        do {
            if(strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..")) {
                continue;
            }

            snprintf(targetPath, sizeof(targetPath), "%s\\%s", dirPath, findFileData.cFileName);

            if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                char* directoryName = strrchr(targetPath, '\\');
                if(directoryName != NULL) {
                    directoryName++;
                    directories = (char**) realloc(directories, (count+1) * sizeof(char*));
                    directories[count] = strdup(directoryName);
                    count++;
                }
            } 
        } while(FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
        directories = (char**) realloc(directories, (count+1) * sizeof(char*));
        directories[count] = NULL;
        
    #else
        struct dirent *entry;
        DIR *dir = opendir(dirPath);

        if(dir == NULL) {
            return 0;
        }

        while((entry = readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char targetPath[PATH_MAX];
            snprintf(targetPath, sizeof(targetPath), "%s/%s", dirPath, entry->d_name);

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
        directories = (char**) realloc(directories, (count+1) * sizeof(char*));
        directories[count] = NULL;

    #endif
    return directories;
}


__attribute__((visibility("hidden"))) 
char** list_files(const char* dirPath, int option) {
    char** files = NULL;
    int count=0;
    #if defined(_WIN32) || defined(_WIN64)
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        char targetPath[MAX_PATH];

        snprintf(targetPath, sizeof(targetPath), "%s\\*", dirPath);
        hFind = FindFirstFile(targetPath, &findFileData);

        if(hFind == INVALID_HANDLE_VALUE) {
            return;
        }

        do {
            if(strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..")) {
                continue;
            }

            snprintf(targetPath, sizeof(targetPath), "%s\\%s", dirPath, findFileData.cFileName);

            if(!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                char* fileName = findFileData.cFileName;
                files = (char**) realloc(files, (count+1) * sizeof(char*));
                files[count] = strdup(fileName);
                count++;

                free(fileName);
            } 
        } while(FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
        files = (char**) realloc(files, (count+1) * sizeof(char*));
        files[count] = NULL;
        
    #else
        struct dirent *entry;
        DIR *dir = opendir(dirPath);
        if(dir == NULL) {
            return files;
        }

        while((entry = readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char targetPath[PATH_MAX];
            snprintf(targetPath, sizeof(targetPath), "%s/%s", dirPath, entry->d_name);
            
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
                if(strstr(entry->d_name, VICTO_FILE_EXT) != NULL) {
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
        files = (char**) realloc(files, (count+1) * sizeof(char*));
        files[count] = NULL;

    #endif
    return files;
}

__attribute__((visibility("hidden"))) 
char* remove_file_extension(const char* fileName) {
    #if defined(_WIN32) || defined(_WIN64)
        char baseName[MAX_PATH];
        pathFindFileName(fileName);
        PathRemoveExtension(fileName);
        return strdup(fileName);
    #else
        char* baseName = basename((char *) fileName);
        char* dotPosition = strrchr(baseName, '.');
        if(dotPosition != NULL) {
            *dotPosition = '\0';
        }
        return strdup(baseName);

    #endif
}

__attribute__((visibility("hidden"))) 
int get_victo_files_count(const char* dirPath) {
    int count=0;
    #if defined(_WIN32) || defined(_WIN64)
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = INVALID_HANDLE_VALUE;
        char targetPath[MAX_PATH];

        snprintf(targetPath, sizeof(targetPath), "%s\\*", dirPath);
        hFind = FindFirstFile(targetPath, &findFileData);

        if(hFind == INVALID_HANDLE_VALUE) {
            return;
        }

        do {
            if(strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..")) {
                continue;
            }

            snprintf(targetPath, sizeof(targetPath), "%s\\%s", dirPath, findFileData.cFileName);

            if(!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                char* fileName = findFileData.cFileName;
                files = (char**) realloc(files, (count+1) * sizeof(char*));
                files[count] = strdup(fileName);
                count++;

                free(fileName);
            } 
        } while(FindNextFile(hFind, &findFileData) != 0);

        FindClose(hFind);
        files = (char**) realloc(files, (count+1) * sizeof(char*));
        files[count] = NULL;
        
    #else
        struct dirent *entry;
        DIR *dir = opendir(dirPath);
        if(dir == NULL) {
            return count;
        }

        while((entry = readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char targetPath[PATH_MAX];
            snprintf(targetPath, sizeof(targetPath), "%s/%s", dirPath, entry->d_name);
            
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
                if(strstr(entry->d_name, VICTO_FILE_EXT) != NULL) {
                    count++; 
                }
                
            } 
        }
        closedir(dir);
        
    #endif
    return count;
}

__attribute__((visibility("hidden"))) 
int deleteVictoFile(const char* filePath) {
    #if defined(_WIN32) || defined(_WIN64)
        if(DeleteFile(filePath) == 0) {
            return 1;
        } else {
            return 0;
        }
    #else
        if(strstr(filePath, VICTO_FILE_EXT) != NULL) {
            if(remove(filePath) !=  0) {
                return 1;
            } else {
                return 0;
            }
        } else {
            return -1;
        }
        
    #endif
}