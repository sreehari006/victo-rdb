#include "interface/uuid.h"

typedef struct {
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_hi_and_version;
    uint8_t clock_seq_hi_and_reserved;
    uint8_t clock_seq_low;
    uint8_t node[6];
} UUID;

uint32_t random_in_range(uint32_t min, uint32_t max) {
    return min + (rand() % (max - min + 1));
}

void generate_uuid(UUID *this_uuid) {
    this_uuid->time_hi_and_version = (uint16_t)((rand() & 0xFFFF) | 0x4000);
    this_uuid->clock_seq_hi_and_reserved = (uint8_t)((rand() & 0xFF) | 0x80);
    this_uuid->time_low = random_in_range(1, 0xFFFFFFFF);
    this_uuid->time_mid = random_in_range(0, 0xFFFF);
    this_uuid->clock_seq_low = (uint8_t)random_in_range(0, 0xFF);

    for (int i = 0; i < 6; ++i) {
        this_uuid->node[i] = (uint8_t)random_in_range(0, 0xFF);
    }
}

long long current_time_nanoseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000000000 + ts.tv_nsec;
}


char* getUUID() {
    srand((unsigned int) current_time_nanoseconds());

    UUID this_uuid;
    generate_uuid(&this_uuid);

    char* formatted_uuid_copy = (char*) malloc(37);
    snprintf(formatted_uuid_copy,37,
        "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
        this_uuid.time_low, this_uuid.time_mid, this_uuid.time_hi_and_version,
        this_uuid.clock_seq_hi_and_reserved, this_uuid.clock_seq_low,
        this_uuid.node[0], this_uuid.node[1], this_uuid.node[2],
        this_uuid.node[3], this_uuid.node[4], this_uuid.node[5]
    );

    return formatted_uuid_copy; 
}
