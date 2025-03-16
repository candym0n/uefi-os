#ifndef FORMAT_H
#define FORMAT_H

#include <stdio.h>
#include <stdbool.h>
#include <uchar.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <fs/candyfs/candyfs.h>
#include <common/crc32.h>
#include "helpers.h"

// Format that image!
bool format_image_cfs(FILE *image, uint64_t image_size, char16_t name[16]);

#endif
