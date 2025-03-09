#include <common/random.h>

// Shush GCC
#pragma GCC diagnostic ignored "-Woverflow"

static uint32_t next = 1;

/* Set the seed for the random number generator */
void srand(uint32_t seed) {
    next = seed;
}

/* Generate a pseudo-random integer between 0 and RAND_MAX */
int rand(void) {
    next = next * 1103515245 + 12345;
    return (uint32_t)(next / 65536) % (RAND_MAX + 1);
}
