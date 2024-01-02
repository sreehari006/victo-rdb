#include <stdbool.h>

bool fileExists(const char* filename);
bool dirExists(const char* dirPath);
// void delete_files_and_subdirectories(const char* dirPath);
int get_directory_count(const char* dirPath);
char** list_directory(const char* dirPath);
char** list_files(const char* dirPath, int option);
char* remove_file_extension(const char* fileName);
int get_victo_files_count(const char* dirPath);
int deleteVictoFile(const char* filePath);
int delete_victo_collection(const char* dirPath);