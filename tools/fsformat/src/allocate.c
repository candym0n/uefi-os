/**
 * TODO: This implementation is anything but efficient.
 * It uses only 1 extent so unless you only added files it is going to be very fragmented
 * 
 * TODO: Right now there is no way to recover from or know about a fatality after closing fsformtat
 */

#include <allocate.h>
#include <common/crc32.h>

static bool find_contiguous_zeros(uint8_t *bitmap, uint64_t count, uint64_t group_size, uint64_t *start_index) {
    uint64_t blocks_found = 0;
    bool have_start = false;

    for (uint64_t i = 0; i < BLOCK_SIZE && blocks_found < count; ++i) {
        uint8_t byte = bitmap[i];

        // Go through each bit in the byte
        for (int j = 0; j < 8; ++j) {
            uint64_t block_index = i * 8 + j;
            uint8_t bit = (byte >> j) & 0x1;

            if (block_index >= group_size) {
                fprintf(stderr, "Not enough contiguous free blocks available in group \n");
                return false;
            }

            if (bit == 0) { // Free block
                if (!have_start) {
                    have_start = true;
                    *start_index = block_index;
                    blocks_found = 1;
                } else {
                    ++blocks_found;
                }
            } else { // Allocated block, reset run
                have_start = false;
                blocks_found = 0;
            }
        }
    }

    if (blocks_found < count) {
        fprintf(stderr, "Not enough contiguous free blocks available \n");
        return false;
    }

    return true;
}

/**
 * Allocate `count` blocks in the block group `group` and return a SINGLE extent describing the allocation.
 * Steps:
 *  1. Read the superblock to get the block group descriptor table location and size
 *  2. Read the group descriptor for the block group
 *  3. Read the block bitmap for the block group
 *  4. Find `count` contiguous free blocks in the bitmap
 *  5. Mark those blocks as allocated in the bitmap and group descriptor
 */

bool allocate_blocks_cfs(FILE *image, uint64_t group, uint64_t count, cfs_extent_t *extent)
{
    if (!image || !extent || count == 0) {
        fprintf(stderr, "Invalid parameters for block allocation.\n");
        return false;
    }

    // --- Read the superblock ---
    cfs_superblock_t sb;
    if (fseek(image, 0, SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed for superblock");
        return false;
    }
    if (fread(&sb, sizeof(sb), 1, image) != 1) {
        fprintf(stderr, "Failed to read superblock\n");
        return false;
    }

    uint64_t gdt_start = sb.gdt_start;
    uint64_t group_size = sb.block_group_size;
    
    // --- Read the group descriptor ---
    cfs_group_desc_t gd;
    uint64_t gd_offset = gdt_start * BLOCK_SIZE + group * sizeof(cfs_group_desc_t);
    if (fseek(image, gd_offset, SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed for group descriptor");
        return false;
    }
    if (fread(&gd, sizeof(cfs_group_desc_t), 1, image) != 1) {
        fprintf(stderr, "Failed to read group descriptor for group %lu\n", group);
        return false;
    }

    // Check if the block bitmap is initialized
    if (gd.block_bitmap == 0) {
        fprintf(stderr, "Block bitmap for group %lu is uninitialized\n", group);
        return false;
    }

    // Check if we even have a chance
    if (gd.free_blocks < count) {
        fprintf(stderr, "Not enough free blocks in group %lu\n", group);
        return false;
    }

    // --- Read the block bitmap ---
    uint8_t bitmap[BLOCK_SIZE];
    uint64_t bb_offset = gd.block_bitmap * BLOCK_SIZE;
    if (fseek(image, bb_offset, SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed for block bitmap");
        return false;
    }
    if (fread(bitmap, BLOCK_SIZE, 1, image) != 1) {
        fprintf(stderr, "Failed to read block bitmap for group %lu\n", group);
        return false;
    }

    // --- Find free blocks in the bitmap ---
    uint64_t first_block;

    if (!find_contiguous_zeros(bitmap, count, group_size, &first_block))
        return false; // Error message already printed by find_contiguous_zeros

    // --- Build the extent ---
    extent->logical_block = 0; // This extent will take up the entire filesystem object
    extent->count = count;
    extent->physical_block = group * group_size + first_block;

    // --- Mark blocks as allocated in the bitmap ---
    for (uint64_t i = first_block; i < first_block + count; ++i) {
        uint64_t byte_index = i / 8;
        uint64_t bit_index = i % 8;
        
        bitmap[byte_index] |= (1 << bit_index); // Mark block as allocated
    }

    if (fseek(image, gd.block_bitmap * BLOCK_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed for block bitmap. Fatal. Your filesystem is now corrupt.");
        return false;
    }

    if (fwrite(bitmap, BLOCK_SIZE, 1, image) != 1) {
        fprintf(stderr, "Failed to write updated block bitmap for group %lu. Fatal. Your filesystem is now corrupt.\n", group);
        return false;
    }

    // --- Update the Group Descriptor ---
    gd.free_blocks -= count;
    gd.checksum = 0; // Clear checksum before recalculating
    gd.checksum = crc32_calculate(&gd, sizeof(cfs_group_desc_t));

    if (fseek(image, gd_offset, SEEK_SET) != 0) {
        fprintf(stderr, "fseek failed for group descriptor. Fatal. Your filesystem is now corrupt.");
        return false;
    }

    if (fwrite(&gd, sizeof(cfs_group_desc_t), 1, image) != 1) {
        fprintf(stderr, "Failed to write updated group descriptor for group %lu. Fatal. Your filesystem is now corrupt.\n", group);
        return false;
    }

    return true;
}
