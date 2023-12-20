#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>


#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define WEBSOCK_PORT 2018

void startWebSockServer();
void stopWebSockServer();