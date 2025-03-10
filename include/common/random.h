#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

#ifndef RAND_MAX
#define RAND_MAX 2147483647 // Default value
#endif

/**
 * @brief Set the seed for the random number generator
 * 
 * @param seed The seed to set
 * @return Nothing (void)
 */
extern void srand(uint32_t seed);

/**
 * @brief Generate a pseudo-random integer between 0 and RAND_MAX
 *
 * @return The generated random number
 */
extern int rand(void);

#endif // RANDOM_H
