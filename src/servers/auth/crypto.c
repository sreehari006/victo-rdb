#include "./includes/crypto_proto.h"
#include "../../utils/logs/includes/log_proto.h"

char* vt__sha256(const char *input) {
    vt__log_writer(LOG_DEBUG, "crypto sha256 started");

    EVP_MD_CTX *mdctx;
    unsigned char *hash = malloc(SHA256_DIGEST_LENGTH);

    if (hash == NULL) {
        vt__log_writer(LOG_DEBUG, "Memory allocation failed while allocating memory for sha256 hash");
        return NULL;
    }

    if ((mdctx = EVP_MD_CTX_new()) == NULL) {
        vt__log_writer(LOG_DEBUG, "Error creating context for sha256");
        return NULL;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        vt__log_writer(LOG_DEBUG, "Error initializing digest for sha256");
        return NULL;
    }

    if (EVP_DigestUpdate(mdctx, input, strlen(input)) != 1) {
        vt__log_writer(LOG_DEBUG, "Error updating digest for sha256");
        return NULL;
    }

    if (EVP_DigestFinal_ex(mdctx, hash, NULL) != 1) {
        vt__log_writer(LOG_DEBUG, "Error finalizing digest for sha256");
        return NULL;
    }

    char *hexHash = malloc(2 * SHA256_DIGEST_LENGTH + 1); 
    if (hexHash == NULL) {
        free(hash);
        EVP_MD_CTX_free(mdctx);
        vt__log_writer(LOG_DEBUG, "Memory allocation failed while allocating memory for sha256 hex hash");
        return NULL;
    }

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hexHash + (i * 2), "%02x", hash[i]);
    }

    hexHash[2 * SHA256_DIGEST_LENGTH] = '\0';

    free(hash);
    EVP_MD_CTX_free(mdctx);
    vt__log_writer(LOG_DEBUG, "crypto sha256 completed");

    return hexHash;
}

void vt__base64_decode(const char *input, char **output, size_t *output_len) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input, -1); 

    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    *output_len = bufferPtr->length;

    *output = (char *)malloc(*output_len + 1);

    int bytes_read = BIO_read(bio, *output, *output_len);

    (*output)[bytes_read] = '\0';

    BIO_free_all(bio);
}