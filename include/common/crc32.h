#ifndef CRC32_H
#define CRC32_H

#include <common/types.h>

/**
 * @brief Calculate CRC32 for a buffer
 * 
 * @param data Pointer to data buffer
 * @param length Length of data in bytes
 * @return uint32_t Calculated CRC32 value
 */
uint32_t crc32_calculate(const void *data, size_t length);

/**
 * @brief Calculate CRC32 with an initial value (for incremental CRC calculation)
 * 
 * @param crc Initial CRC value
 * @param data Pointer to data buffer
 * @param length Length of data in bytes
 * @return uint32_t Updated CRC32 value
 */
uint32_t crc32_update(uint32_t crc, const void *data, size_t length);

/**
 * @brief Verify if data matches expected CRC32
 * 
 * @param data Pointer to data buffer
 * @param length Length of data in bytes
 * @param expected_crc Expected CRC32 value
 * @return int 1 if CRC matches, 0 if not
 */
int crc32_verify(const void *data, size_t length, uint32_t expected_crc);

#endif // CRC32_H
