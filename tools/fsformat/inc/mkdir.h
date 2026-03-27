#ifndef MKDIR_H
#define MKDIR_H

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <fs/candyfs/candyfs.h>
#include <common/crc32.h>
#include <common/string.h>

/**
 * Create a new directory
 * 
 * @param image  Opened file handle to the filesystem image.
 * @param parent Parent directory's inode number
 * @param mode   Permissions and type for the new directory.
 * @param user   User ID for the new directory.
 * @param group  Group ID for the new directory.
 * 
 * @return true on success, false on failure.
 */
bool mkdir_cfs(FILE *image, uint64_t parent, cfs_file_mode_t mode, uint16_t user, uint16_t group);

#endif // MKDIR_H
