#ifndef FORMAT_H
#define FORMAT_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <fs/candyfs/candyfs.h>
#include <common/crc32.h>
#include <common/string.h>
#include "helpers.h"

/**
 * Format a new CFS filesystem image.
 * 
 * @param image       Opened file handle to write into.
 * @param image_size  Total capacity in bytes of the image.
 * @param name        Filesystem label (UTF-16, 16 chars max).
 * 
 * @return true on success, false on failure.
 */
bool format_image_cfs(FILE *image, uint64_t image_size, char16_t name[16]);

#endif
