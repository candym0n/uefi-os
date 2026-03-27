#include <mkdir.h>
#include <allocate.h>
#include <common/crc32.h>

const uint64_t HARDCODED_GROUP = 0; // TODO: Don't hardcode this

/**
 * Create a new directory in the filesystem.
 * Steps:
 *  1. 
 */

bool mkdir_cfs(FILE *image, uint64_t parent, uint64_t inode, cfs_file_mode_t mode, uint16_t user, uint16_t group)
{
    if (!image) {
        fprintf(stderr, "Invalid image.\n");
        return false;
    }

    // --- Initialize the directory's inode ---
    cfs_inode_t dir_inode = {0};

    dir_inode.mode = mode;
    dir_inode.user = user;
    dir_inode.group = group;
    dir_inode.hard_links_count = 2; // "." and ".."
    dir_inode.creation_time = dir_inode.modification_time = dir_inode.access_time = time(NULL);
    dir_inode.byte_size = 0;
    dir_inode.block_count = 0;
    
    dir_inode.extents.extents.header.magic = EXTENT_MAGIC;
    dir_inode.extents.extents.header.entries = 1;
    dir_inode.extents.extents.header.depth = 1;

    if (!allocate_blocks_cfs(image, HARDCODED_GROUP, 1, &dir_inode.extents.extents.tree.extents[0])) {
        fprintf(stderr, "Failed to allocate blocks");
        return false;
    }

    dir_inode.checksum = crc32_calculate(&dir_inode, sizeof(cfs_inode_t));

    if (fseek(image, INODE_TABLE_BLOCK(0) * BLOCK_SIZE + inode * sizeof(cfs_inode_t), SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed");
        return false;
    }

    if (fwrite(&dir_inode, sizeof(cfs_inode_t), 1, image) != 1) {
        fprintf(stderr, "Failed to write directory inode for inode %lu\n", inode);
        return false;
    }

    // --- Add '.' and '..' entries to the directory block ---
    cfs_dir_entry_t entries[2] = {0};
    strcpy(entries[0].name, ".");
    entries[0].inode = inode;
    strcpy(entries[1].name, "..");
    entries[1].inode = parent;

    if (fseek(image, dir_inode.extents.extents.tree.extents[0].physical_block * BLOCK_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed");
        return false;
    }

    if (fwrite(entries, sizeof(cfs_dir_entry_t), 2, image) != 2) {
        fprintf(stderr, "Failed to write directory entries for inode %lu\n", inode);
        return false;
    }

    // --- Update the superblock ---
    rewind(image);

    cfs_superblock_t sb = {0};

    if (fread(&sb, BLOCK_SIZE, 1, image) != 1) {
        fprintf(stderr, "Failed to read superblock\n");
        return false;
    }

    sb.free_blocks_count -= 1;
    sb.free_inodes_count -= 1;

    sb.checksum = crc32_calculate(&sb, BLOCK_SIZE);

    if (fseek(image, 0, SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed");
        return false;
    }

    if (fwrite(&sb, BLOCK_SIZE, 1, image) != 1) {
        fprintf(stderr, "Failed to write superblock\n");
        return false;
    }

    // --- Update the group descriptor ---
    cfs_group_desc_t gd = {0};

    if (fseek(image, BLOCK_SIZE + HARDCODED_GROUP * sizeof(cfs_group_desc_t), SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed");
        return false;
    }

    gd.dir_count += 1;
    gd.free_inodes -= 1;

    return true;
}