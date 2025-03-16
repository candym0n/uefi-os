#ifndef HELPERS_H
#define HELPERS_H

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>

// Set a UUID to random
void rand_uuid(uint8_t uuid[16]);

// Set a range of bits starting at `bit`
void set_bits(uint8_t *bitmap, uint64_t bit, uint64_t span);

// Clear a range of bits starting at `bit`
void clear_bits(uint8_t *bitmap, uint64_t bit, uint64_t span);

#endif // HELPERS_H
