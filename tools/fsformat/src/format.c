#include <common/string.h>
#include "format.h"

/**
 * Format a new CFS filesystem image.
 * Steps:
 *  1. Compute fundamental layout (block counts, GDT size, offsets)
 *  2. Initialize the superblock with default values
 *  3. Build the group descriptor table
 *  4. Initialize all bitmaps for every block group
 *  5. Create and write the root inode
 */
bool format_image_cfs(FILE *image, uint64_t image_size, char16_t name[CFS_SB_NAME_LEN]) {
    if (!image || image_size < BLOCK_SIZE) {
        fprintf(stderr, "Invalid image or size.\n");
        return false;
    }

    // Calculate basic layout parameters
    const uint32_t blocks = (image_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    const uint32_t block_groups = (blocks + GROUP_SIZE - 1) / GROUP_SIZE;
    const uint32_t gdt_span = 
        (block_groups * sizeof(cfs_group_desc_t) + BLOCK_SIZE - 1) / BLOCK_SIZE;

    const uint64_t block_bitmap_start = 2 + gdt_span;
    const uint64_t inode_bitmap_start = 3 + gdt_span;
    const uint64_t inode_table_start  = 4 + gdt_span;

    rewind(image);

    // --- Initialize and write superblock ---
    cfs_superblock_t sb = {0};
    rand_uuid(sb.uuid);
    memcpy(sb.name, name, sizeof(char16_t) * CFS_SB_NAME_LEN);
    sb.magic               = SB_MAGIC;
    sb.inodes_count        = INODES_PER_GROUP * block_groups;
    sb.blocks_count        = block_groups * GROUP_SIZE;
    sb.group_count         = block_groups;
    sb.free_inodes_count   = sb.inodes_count - 1; // root inode reserved
    sb.free_blocks_count   = blocks - block_groups * (2 + CFS_IT_SPAN) - 1 - gdt_span;
    sb.block_size          = BLOCK_SIZE;
    sb.block_group_size    = GROUP_SIZE;
    sb.reserved_inodes     = RSVD_INODES;
    sb.bad_blks_inode_index= BAD_BLKS_INODES;
    sb.root_inode_index    = ROOT_INODE;
    sb.inodes_per_group    = INODES_PER_GROUP;
    sb.inode_size          = sizeof(cfs_inode_t);
    sb.gdt_start           = 1;
    sb.gdt_span            = gdt_span;
    sb.creation_time       = time(NULL);
    sb.tree_order          = CFS_TREE_ORDER;
    sb.checksum            = crc32_calculate(&sb, BLOCK_SIZE);

    if (fwrite(&sb, BLOCK_SIZE, 1, image) != 1) {
        perror("Superblock write failed");
        return false;
    }

    // --- Initialize and write GDT entries ---
    for (uint64_t i = 0; i < block_groups; ++i) {
        cfs_group_desc_t gd = {0};
        gd.block_bitmap = (i == 0) ? block_bitmap_start : BLOCK_BITMAP_BLOCK(i);
        gd.inode_bitmap = (i == 0) ? inode_bitmap_start : INODE_BITMAP_BLOCK(i);
        gd.inode_table  = (i == 0) ? inode_table_start  : INODE_TABLE_BLOCK(i);
        gd.free_inodes  = INODES_PER_GROUP - ((i == 0) ? 1 : 0);
        gd.free_blocks  = GROUP_SIZE - CFS_IT_SPAN - 2;
        gd.dir_count    = (i == 0) ? 1 : 0;
        gd.checksum     = crc32_calculate(&gd, sizeof(cfs_group_desc_t));

        if (fwrite(&gd, sizeof(cfs_group_desc_t), 1, image) != 1) {
            fprintf(stderr, "Failed writing GDT entry #%lu\n", i);
            return false;
        }
    }

    // --- Initialize bitmaps ---
    uint8_t *bitmap = calloc(1, BLOCK_SIZE);
    if (!bitmap) {
        fprintf(stderr, "Memory allocation failed.\n");
        return false;
    }

    for (uint64_t i = 0; i < block_groups; ++i) {
        // Block bitmap
        uint64_t bb_block = (i == 0) ? block_bitmap_start : BLOCK_BITMAP_BLOCK(i);
        if (fseek(image, bb_block * BLOCK_SIZE, SEEK_SET) != 0) {
            perror("fseek failed");
            free(bitmap);
            return false;
        }
        memset(bitmap, 0, BLOCK_SIZE);
        if (fwrite(bitmap, BLOCK_SIZE, 1, image) != 1) {
            fprintf(stderr, "Failed writing block bitmap #%lu\n", i);
            free(bitmap);
            return false;
        }

        // Inode bitmap
        uint64_t ib_block = (i == 0) ? inode_bitmap_start : INODE_BITMAP_BLOCK(i);
        if (fseek(image, ib_block * BLOCK_SIZE, SEEK_SET) != 0) {
            perror("fseek failed");
            free(bitmap);
            return false;
        }
        memset(bitmap, 0, BLOCK_SIZE);
        if (i == 0) set_bits(bitmap, 0, RSVD_INODES); // reserve root + system inodes
        if (fwrite(bitmap, BLOCK_SIZE, 1, image) != 1) {
            fprintf(stderr, "Failed writing inode bitmap #%lu\n", i);
            free(bitmap);
            return false;
        }
    }

    free(bitmap);

    // --- Initialize root inode ---
    cfs_inode_t root_inode = {0};
    root_inode.mode.type                = DIRECTORY;
    root_inode.mode.owner_permissions   = READ | WRITE | EXECUTE;
    root_inode.mode.group_permissions   = READ | EXECUTE;
    root_inode.mode.others_permissions  = READ | EXECUTE;
    root_inode.user                     = ROOT_USER_UUID;
    root_inode.group                    = ROOT_GROUP_UUID;
    root_inode.hard_links_count         = 0;
    root_inode.creation_time            = root_inode.modification_time =
                                           root_inode.access_time = time(NULL);
    root_inode.byte_size                = 0;
    root_inode.block_count              = 0;
    root_inode.checksum                 = crc32_calculate(&root_inode, sizeof(cfs_inode_t));

    if (fseek(image, inode_table_start * BLOCK_SIZE, SEEK_SET) != 0) {
        perror("fseek failed");
        return false;
    }

    if (fwrite(&root_inode, sizeof(cfs_inode_t), 1, image) != 1) {
        perror("Failed to write root inode");
        return false;
    }

    return true;
}
