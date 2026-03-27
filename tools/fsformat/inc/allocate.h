#ifndef ALLOCATE_H
#define ALLOCATE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fs/candyfs/candyfs.h>

/**
 * Allocate some number of blocks, change bitmaps, and return the extent via pointer
 * 
 * @param image Opened file handle to the filesystem image.
 * @param group The block group to allocate from.
 * @param count The number of blocks to allocate.
 * @param extent Pointer to an extent structure to fill with the allocated blocks.
 * @return true on success, false on failure.
 */
bool allocate_blocks_cfs(FILE *image, uint64_t group, uint64_t count, cfs_extent_t *extent);

/**
 * Allocate an inode with some number of blocks as data at a specific inode
 * 
 * @param image  Opened file handle to the filesystem image.
 * @param parent Parent inode number
 * @param inode  Inode number to allocate
 * @param size   Size of the data in bytes
 */
bool allocate_inode_cfs(FILE *image, uint64_t parent, uint64_t inode, uint64_t size)

#endif // ALLOCATE_H
