#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Copies n bytes from memory area src to memory area dest.
 *
 * @param dest A pointer to the destination memory area where the content is to be copied.
 * @param src A pointer to the source memory area from which content is to be copied.
 * @param n The number of bytes to be copied.
 * @return A pointer to the destination memory area dest.
 */
extern void *memcpy(void *dest, const void *src, size_t n);

/**
 * @brief Compares two blocks of memory
 *
 * @param ptr1 Pointer to first block of memory to compare
 * @param ptr2 Pointer to second block of memory to compare
 * @param n Number of bytes to compare
 * @return
 *   - < 0 if the first differing byte has a lower value in ptr1 than in ptr2
 *   - 0 if all n bytes are identical
 *   - > 0 if the first differing byte has a greater value in ptr1 than in ptr2
 */
extern int memcmp(const void* ptr1, const void* ptr2, size_t n);

#endif // MEMORY_H
