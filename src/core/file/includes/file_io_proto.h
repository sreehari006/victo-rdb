int get_directory_count(const char* dir_path);
char** list_directory(const char* dir_path);
char** list_files(const char* dir_path, int option, const char* ext);
char* remove_file_extension(const char* file_name);
int get_victo_files_count(const char* dir_path, const char* ext);
int delete_victo_file(const char* file_path);
int delete_victo_collection(const char* dir_path);
int delete_subscription(const char* file_path);