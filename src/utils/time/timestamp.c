#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

void timestampToString(char *buffer, size_t bufferSize) {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    time_t seconds = currentTime.tv_sec;
    long milliseconds = currentTime.tv_usec / 1000;

    struct tm *localTime = localtime(&seconds);

    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", localTime);

    snprintf(buffer + strlen(buffer), bufferSize - strlen(buffer), ".%03ld", milliseconds);
}