#include <common/string.h>
#include "format.h"

/**
 * 1. Calculate the offsets and values of things (bitmaps, # of block groups, size of GDT, etc)
 * 2. Initialize the superblock by filling in default values
 * 3. Initialize all of the bitmaps for all of the the block groups
 * 4. Initialize all GDT entries
 * 5. Write default values for the root inode
 */
bool format_image_cfs(FILE *image, uint64_t image_size, char16_t name[CFS_SB_NAME_LEN]) {
    // Calculate the number of blocks the GDT spans
    uint32_t blocks = (image_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint32_t block_groups = (blocks + GROUP_SIZE - 1) / (GROUP_SIZE);
    uint32_t gdt_span =
        (block_groups * sizeof(cfs_group_desc_t) / BLOCK_SIZE) +
        ((block_groups * sizeof(cfs_group_desc_t) % BLOCK_SIZE != 0) ? 1 : 0);

    // Calculate the location of metadata specific to the first block group
    uint64_t block_bitmap_start = 2 + gdt_span; // After superblock and GDT
    uint64_t inode_bitmap_start = 3 + gdt_span; // Right after the block bitmap
    uint64_t inode_table_start = 4 + gdt_span;  // Right after the inode bitmap

    // Reset the file pointer to get ready for writing :)
    rewind(image);

    /*
     * Write the superblock to the first block
     * of the image
     */
    cfs_superblock_t sb = {0};
    rand_uuid(sb.uuid);
    for (int i = 0; i < CFS_SB_NAME_LEN; ++i)
        sb.name[i] = name[i];
    sb.magic = SB_MAGIC;

    sb.inodes_count = INODES_PER_GROUP * block_groups;
    sb.blocks_count = block_groups * GROUP_SIZE; // For now only count blocks part of full groups
    sb.group_count = block_groups;               // Again, only full groups count (right now)

    sb.free_inodes_count = INODES_PER_GROUP * block_groups - 1; // Minus 1 for the root inode
    sb.free_blocks_count = blocks - block_groups * (2 +         // Bitmaps
                                                    CFS_IT_SPAN // Inode table
                                                    ) -
                           1 - gdt_span; // Superblock and GDT

    sb.block_size = BLOCK_SIZE;
    sb.block_group_size = GROUP_SIZE;

    sb.reserved_inodes = RSVD_INODES;
    sb.bad_blks_inode_index = BAD_BLKS_INODES;
    sb.root_inode_index = ROOT_INODE;

    sb.inodes_per_group = INODES_PER_GROUP;
    sb.inode_size = sizeof(cfs_inode_t);

    sb.gdt_start = 1; // First block is the superblock
    sb.gdt_span = gdt_span;

    sb.creation_time = time(NULL);

    sb.tree_order = CFS_TREE_ORDER;

    sb.checksum = crc32_calculate(&sb, BLOCK_SIZE);

    int times = 0;
    uint32_t prev = 0;

    if (fwrite(&sb, BLOCK_SIZE, 1, image) != 1) {
        printf("Failed to write CFS superblock!\n");
        return false;
    }

    /*
     * Initialize all of the block groups' metadata
     */

    // Write entries in the GDT
    for (uint64_t i = 0; i < block_groups; ++i) {
        // Fill out the block group descriptor
        cfs_group_desc_t gd = {0};
        gd.block_bitmap = i == 0 ? block_bitmap_start : BLOCK_BITMAP_BLOCK(i);
        gd.inode_bitmap = i == 0 ? inode_bitmap_start : INODE_BITMAP_BLOCK(i);
        gd.inode_table = i == 0 ? inode_table_start : INODE_TABLE_BLOCK(i);
        gd.free_inodes = INODES_PER_GROUP - (i == 0 ? 1 : 0); // Subtract one for root inode (if the first block group)
        gd.free_blocks = BLOCK_SIZE - CFS_IT_SPAN - 2;        // Take away the inode table and 2 bitmaps
        gd.dir_count = i == 0 ? 1 : 0;                        // Root inode for the first block group
        gd.checksum = crc32_calculate(&gd, sizeof(cfs_group_desc_t));

        // Write it!
        if (fwrite(&gd, sizeof(cfs_group_desc_t), 1, image) != 1)
        {
            printf("Failed to write GDT Entry #%lu!\n", i);
            return false;
        }

        // Go to the next entry
        fseek(image, sizeof(cfs_group_desc_t), SEEK_CUR);
    }

    // Initialize bitmaps to 0 (except for the first block group, which will have reserved inodes)
    for (uint64_t i = 0; i < block_groups; ++i) {
        // Seek to the start of the bitmaps
        fseek(image, (i == 0 ? block_bitmap_start : BLOCK_BITMAP_BLOCK(i)) * BLOCK_SIZE, SEEK_SET);

        // Initialize the bitmap
        uint8_t *bitmap = malloc(BLOCK_SIZE);
        clear_bits(bitmap, 0, BLOCK_SIZE);

        // Write the block bitmap
        if (fwrite(bitmap, BLOCK_SIZE, 1, image) != 1)
        {
            printf("Failed to write block bitmap for block group #%lu!\n", i);
            return false;
        }

        //      fseek(image, (i == 0 ? inode_bitmap_start : INODE_BITMAP_BLOCK(i)) * BLOCK_SIZE, SEEK_SET); // Inode bitmap should follow block bitmap (uncomment this line if not)

        // Reserve the reserved inodes if 'tis the first block group
        if (i == 0)
            set_bits(bitmap, 0, RSVD_INODES);

        // Write the inode bitmap
        if (fwrite(bitmap, BLOCK_SIZE, 1, image) != 1) {
            printf("Failed to write inode bitmap for block group #%lu!\n", i);
            return false;
        }
    }

    // Initialize the root inode
    cfs_inode_t root_inode = {0};

    root_inode.mode.type = DIRECTORY;
    root_inode.mode.owner_permissions = READ | WRITE | EXECUTE;
    root_inode.mode.group_permissions = READ | 0 | EXECUTE;
    root_inode.mode.others_permissions = READ | 0 | EXECUTE;

    root_inode.user = ROOT_USER_UUID;
    root_inode.group = ROOT_GROUP_UUID;

    root_inode.hard_links_count = 0; // This is the root inode, dummy!

    root_inode.creation_time = time(NULL);
    root_inode.modification_time = time(NULL); // Technically creation = modification...
    root_inode.access_time = time(NULL);       // You have to access it to create it (?)

    root_inode.byte_size = 0;   // Initially the root inode will be completely empty
    root_inode.block_count = 0; // Again, empty

    root_inode.reserved = 0;

    root_inode.checksum = crc32_calculate(&root_inode, sizeof(cfs_inode_t));

    // Write it!
    fseek(image, inode_table_start * BLOCK_SIZE, SEEK_SET);

    if (fwrite(&root_inode, sizeof(cfs_inode_t), 1, image) != 1) {
        printf("Failed to initialize the root inode!\n");
        return false;
    }

    /* TODO: Initialize inode table for other blocks(?) */
    return true;
}
