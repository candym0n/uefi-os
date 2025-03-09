#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <common/types.h>
#include <common/string.h>

// The size of the configuration part of the configuration file
#define CONFIG_SIZE (2+8+4+1+8)

// A number that fits into any sector / LBA block size
#define INTERNAL_UNIT 512

// The alignment value (starts and ends of partitionsmust be a multiple of this value)
#define ALIGNMENT 1024 * 1024 // Megabyte

// The alignment value in LBA blocks
#define ALIGN_LBA (ALIGNMENT / lba_size)

// Global variable(s) (don't get mad)
extern uint32_t lba_size;

#endif
