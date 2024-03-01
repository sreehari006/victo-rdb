#include <sys/time.h>
#include <time.h>

void vt__timestamp_to_string(char *buffer, int buffer_size);
void vt__gettimeoftheday(struct timeval* timeval);
char* vt__elapsedtime(struct timeval* start, struct timeval* end, double metric);
