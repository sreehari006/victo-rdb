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

// Define ntohll if it's not already provided                                                                                               
#ifndef ntohll                                                                                                                              
#if __BYTE_ORDER == __BIG_ENDIAN                                                                                                            
#define ntohll(x)   (x)                                                                                                                     
#define htonll(x)   (x)                                                                                                                     
#elif __BYTE_ORDER == __LITTLE_ENDIAN                                                                                                       
#define ntohll(x)   ((((uint64_t)ntohl(x)) << 32) + ntohl(x >> 32))                                                                         
#define htonll(x)   ((((uint64_t)htonl(x)) << 32) + htonl(x >> 32))                                                                         
#else                                                                                                                                       
#error "Unknown byte order"                                                                                                                 
#endif                                                                                                                                      
#endif 

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define WEBSOCK_PORT 8080



void startWebSockServer();
void stopWebSockServer();