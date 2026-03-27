#include <common/string.h>
#include <common/memory.h>
#include <mkdir.h>
#include <format.h>

#define BLOCK_INDEX(block_num) ((block_num) % GROUP_SIZE)

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

    // --- Calculate basic layout parameters ---
    const uint32_t blocks = (image_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    const uint32_t block_groups = (blocks + GROUP_SIZE - 1) / GROUP_SIZE;
    const uint32_t gdt_span = 
        (block_groups * sizeof(cfs_group_desc_t) + BLOCK_SIZE - 1) / BLOCK_SIZE;

    const uint64_t block_bitmap_start = 2 + gdt_span;
    const uint64_t inode_bitmap_start = 3 + gdt_span;
    const uint64_t inode_table_start  = 4 + gdt_span;
    const uint64_t gdt_start = 1;

    rewind(image);

    // --- Initialize and write superblock ---
    cfs_superblock_t sb = {0};
    rand_uuid(sb.uuid);
    memcpy(sb.name, name, sizeof(char16_t) * CFS_SB_NAME_LEN);
    sb.magic               = SB_MAGIC;
    sb.inodes_count        = INODES_PER_GROUP * block_groups;
    sb.blocks_count        = blocks;
    sb.group_count         = block_groups;
    sb.free_inodes_count   = sb.inodes_count - RSVD_INODES;
    sb.free_blocks_count   = blocks - 1 - gdt_span - block_groups * (2 + CFS_IT_SPAN); // Account for superblock, GDT, and bitmaps / IT for each block group
    sb.block_size          = BLOCK_SIZE;
    sb.block_group_size    = GROUP_SIZE;
    sb.reserved_inodes     = RSVD_INODES;
    sb.bad_blks_inode_index= BAD_BLKS_INODES;
    sb.root_inode_index    = ROOT_INODE;
    sb.inodes_per_group    = INODES_PER_GROUP;
    sb.inode_size          = sizeof(cfs_inode_t);
    sb.gdt_start           = gdt_start;
    sb.gdt_span            = gdt_span;
    sb.creation_time       = time(NULL);
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
        gd.free_inodes  = INODES_PER_GROUP - ((i == 0) ? 1 : 0); // Gotta respect the root inode
        gd.free_blocks  = GROUP_SIZE - CFS_IT_SPAN - 2 - ((i == 0) ? 1 + gdt_span : 0); // Account for IT, bitmaps, and superblock + GDT for first group
        gd.dir_count    = (i == 0) ? 1 : 0;
        gd.checksum     = crc32_calculate(&gd, sizeof(cfs_group_desc_t));

        if (fwrite(&gd, sizeof(cfs_group_desc_t), 1, image) != 1) {
            fprintf(stderr, "Failed writing GDT entry #%lu\n", i);
            return false;
        }
    }

    // --- Initialize bitmaps & IT ---
    uint8_t *block_buffer = calloc(1, BLOCK_SIZE);
    if (!block_buffer) {
        fprintf(stderr, "Memory allocation failed.\n");
        return false;
    }

    for (uint64_t i = 0; i < block_groups; ++i) {
        // Get bitmap / IT locations
        uint64_t bb_block = (i == 0) ? block_bitmap_start : BLOCK_BITMAP_BLOCK(i);
        uint64_t ib_block = (i == 0) ? inode_bitmap_start : INODE_BITMAP_BLOCK(i);
        uint64_t it_block = (i == 0) ? inode_table_start  : INODE_TABLE_BLOCK(i);

        // Block bitmap
        if (fseek(image, bb_block * BLOCK_SIZE, SEEK_SET) != 0) {
            fprintf(stderr, "fseek failed");
            free(block_buffer);
            return false;
        }

        memset(block_buffer, 0, BLOCK_SIZE);
    
        set_bits(block_buffer, BLOCK_INDEX(bb_block), 1);
        set_bits(block_buffer, BLOCK_INDEX(ib_block), 1);
        set_bits(block_buffer, BLOCK_INDEX(it_block), 1);

        if (i == 0) {
            set_bits(block_buffer, BLOCK_INDEX(0), 1); // Superblock
            set_bits(block_buffer, BLOCK_INDEX(gdt_start), gdt_span); // GDT
        }

        if (fwrite(block_buffer, BLOCK_SIZE, 1, image) != 1) {
            fprintf(stderr, "Failed writing block bitmap #%lu\n", i);
            free(block_buffer);
            return false;
        }

        // Inode bitmap
        if (fseek(image, ib_block * BLOCK_SIZE, SEEK_SET) != 0) {
            fprintf(stderr, "fseek failed");
            free(block_buffer);
            return false;
        }

        memset(block_buffer, 0, BLOCK_SIZE);
        
        if (i == 0) set_bits(block_buffer, 0, RSVD_INODES); // reserve root + system inodes
        if (fwrite(block_buffer, BLOCK_SIZE, 1, image) != 1) {
            fprintf(stderr, "Failed writing inode bitmap #%lu\n", i);
            free(block_buffer);
            return false;
        }

        // Inode table
        if (fseek(image, it_block * BLOCK_SIZE, SEEK_SET) != 0) {
            fprintf(stderr, "fseek failed");
            free(block_buffer);
            return false;
        }

        memset(block_buffer, 0, BLOCK_SIZE);

        if (fwrite(block_buffer, BLOCK_SIZE, CFS_IT_SPAN, image) != CFS_IT_SPAN) {
            fprintf(stderr, "Failed writing inode table #%lu\n", i);
            free(block_buffer);
            return false;
        }
    }

    free(block_buffer);

    // --- Initialize root inode ---
    cfs_file_mode_t root_inode_mode = {0};
    
    root_inode_mode.type                = DIRECTORY;
    root_inode_mode.owner_permissions   = READ | WRITE | EXECUTE;
    root_inode_mode.group_permissions   = READ | EXECUTE;
    root_inode_mode.others_permissions  = READ | EXECUTE;

    if (!mkdir_cfs(image, ROOT_INODE, root_inode_mode, ROOT_USER_UUID, ROOT_GROUP_UUID)) {
        fprintf(stderr, "Failed to create root directory");
        return false;
    }

    return true;
}
