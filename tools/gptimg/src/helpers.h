#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include "config.h"
#include "gpt.h"

// Pad enough zeros after a sector to fill an LBA block
void pad_lba(FILE *image);

// Calculate CRC32 value for range of data
uint32_t calculate_crc32(void *buf, uint32_t len);

// Fill in the CRC32 table values
void create_crc32_table(void);

// Convert string to number of sectors (e.g 2K = 4096)
uint64_t string_to_sectors(const char *size);

// Create a raondom GUID
guid_t random_guid(void);

// Convert a guid_t to a string of bytes
uint8_t *guid_to_bytes(guid_t guid);

char16_t *ascii_to_ucs2(const char *ascii);

// The CRC32 table to write to
static uint32_t crc_table[256];

#endif
