#include "./includes/timestamp_proto.h"
#include <stdio.h>
#include <string.h>

void vt__timestamp_to_string(char *buffer, int buffer_size) {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    time_t seconds = currentTime.tv_sec;
    long milliseconds = currentTime.tv_usec / 1000;

    struct tm *localTime = localtime(&seconds);

    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", localTime);

    snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), ".%03ld", milliseconds);
}

void vt__gettimeoftheday(struct timeval* timeval) {
    gettimeofday(timeval, NULL);
}

char* vt__elapsedtime(struct timeval* start, struct timeval* end, double metric) {
    double elapsed_time = (end->tv_usec - start->tv_usec) / metric;

    char time_str[20];
    sprintf(time_str, "%.5f", elapsed_time);

    return strdup(time_str);
}