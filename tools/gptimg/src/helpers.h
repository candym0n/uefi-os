#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>
#include <common/string.h>
#include <common/memory.h>
#include <common/random.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include "config.h"
#include "gpt.h"

// Pad enough zeros after a sector to fill an LBA block
void pad_lba(FILE *image);

// Convert string to number of sectors (e.g 2K = 4096)
uint64_t string_to_sectors(const char *size);

// Create a raondom GUID
guid_t random_guid(void);

// Convert a guid_t to a string of bytes
uint8_t *guid_to_bytes(guid_t guid);

char16_t *ascii_to_ucs2(const char *ascii);

#endif
