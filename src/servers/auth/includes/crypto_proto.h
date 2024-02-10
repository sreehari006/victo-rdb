#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

char* vt__sha256(const char *input);
void vt__base64_decode(const char *input, char **output, size_t *output_len);