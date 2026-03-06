#include "helpers.h"

void rand_uuid(uint8_t uuid[16])
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec * 1000000 + tv.tv_usec);
    for (int i = 0; i < 16; ++i)
        uuid[i] = (uint8_t)rand();
}


void set_bits(uint8_t *bitmap, uint64_t bit, uint64_t span) {
    for (uint64_t i = bit; i < bit + span; ++i) {
        uint64_t byte_index = i / 8;
        uint8_t bit_index = i % 8;
        bitmap[byte_index] |= (1 << bit_index);
    }
}

void clear_bits(uint8_t *bitmap, uint64_t bit, uint64_t span) {
    for (uint64_t i = bit; i < bit + span; ++i) {
        uint64_t byte_index = i / 8;
        uint8_t bit_index = i % 8;
        bitmap[byte_index] &= ~(1 << bit_index);
    }
}
