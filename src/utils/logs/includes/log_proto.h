#define LOG_DEBUG                               0
#define LOG_INFO                                1
#define LOG_WARN                                2
#define LOG_ERROR                               3
#define LOG_CRITICAL                            4

void vt__init_log_util(char* log_level, const char* log_path);
void vt__log_writer(const int log_level, const char* message);
void vt__free_log_util();
void vt__set_thread_id_on_register(char* thread_id, char* uuid);
void vt__remove_thread_id_from_register(char* thread_id);
