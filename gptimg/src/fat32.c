#include "fat32.h"

#define MEDIA_NON_REMOVABLE 0xF8
#define MEDIA_FLASH0 xF0

bool format_partition_to_fat32(FILE *image, uint8_t partition_number, uint32_t bytes_per_clus)
{
    // Read the partition table
    fseek(image, GPT_TABLE_START * lba_size, SEEK_SET);
    gpt_partition_entry_t *partition_table = malloc(GPT_TABLE_SIZE);
    if (!partition_table)
    {
        printf("Failed to allocate memory for partition table!\n");
        return false;
    }

    if (fread(partition_table, GPT_TABLE_SIZE, 1, image) != 1) {
        printf("Failed to read the partition table!\n");
        free(partition_table);
        return false;
    }

    // The partition entry for the partition to format
    gpt_partition_entry_t partition = partition_table[partition_number];
    
    // Calculate key values
    uint32_t total_sectors = partition.ending_lba - partition.starting_lba + 1;
    uint32_t sectors_per_cluster = bytes_per_clus / lba_size;
    uint8_t reserved_sectors = 32;  // Standard for FAT32
    uint32_t root_cluster = 2;       // Standard for FAT32

    // Calculate FAT size
    uint32_t total_clusters = total_sectors / sectors_per_cluster;
    uint32_t fat_size = ((total_clusters * 4) + (lba_size - 1)) / lba_size; // Round up

    // Fill out the VBR
    vbr_t vbr = {
        .bs_jmp_boot = {0xEB, 0x58, 0x90},  // JMP start; NOP
        .bs_oem_name = {"MSWIN4.1"},
        .bpb_bytes_per_sec = lba_size,
        .bpb_secs_per_clus = sectors_per_cluster,
        .bpb_rsvd_sec_cnt = reserved_sectors,
        .bpb_num_fats = 2,     // Default
        .bpb_root_ent_cnt = 0, // 0 for FAT32
        .bpb_tot_sec16 = 0,
        .bpb_media = MEDIA_NON_REMOVABLE,
        .bpb_fatsz16 = 0,                               // We don't use FAT 12 / 16!
        .bpb_sec_per_trk = 63,                          // Really only for int 13h which is only for legacy BIOS
        .bpb_num_heads = 255,                           // Also completely useless
        .bpb_hidd_sec_cnt = partition.starting_lba,     // Number of sectors before this partition
        .bpb_tot_sec32 = total_sectors,                 // Size of this partition
        .bpb_fatsz32 = fat_size,                        // Filled out below (scroll down)
        .bpb_flags = 0,                                 // Mirrored FATs
        .bpb_fsver = 0,                                 // No real versions...
        .bpb_root_clus = root_cluster,                  // Clusters 0 and 1 are reserved so root dir cluster starts at 2
        .bpb_fsinfo = 1,                                // Sector 0 = this VBR and FS Info sector follows it
        .bpb_bk_boot_sect = 6,                          // Backup boot sector at sector 6
        .bpb_reserved = {0},
        .bs_drive_num = 0x80, // 1st hard drive
        .bs_reserved = 0,
        .bs_boot_sig = 0x29, // One of the signatures
        .bs_vol_id = {0},
        .bs_vol_lab = {"NO NAME    "},
        .bs_fil_sys_type = {"FAT32   "},
        .boot_code = {0xF4}, // HALT
        .bootsect_sig = 0xAA55
    };

    // Fill out the file system info sector
    fs_info_t fsinfo = {
        .lead_sig = 0x41615252,
        .struct_sig = 0x61417272,
        .reserved1 = {0},
        .free_count = total_clusters - 1,   // Subtract root directory
        .nxt_free = 3,                      // First free cluster after root
        .trail_sig = 0xAA550000
    };
    
    // Write VBR and FSInfo sector to the start of the partition
    fseek(image, partition.starting_lba * (uint64_t)lba_size, SEEK_SET);
    if (fwrite(&vbr, 1, sizeof vbr, image) != sizeof vbr)
    {
        printf("Failed to write VBR to partition %u\n", (uint32_t)partition_number);
        return false;
    }
    pad_lba(image);

    if (fwrite(&fsinfo, 1, sizeof fsinfo, image) != sizeof fsinfo)
    {
        printf("Could not write File System Info Sector to partition %u\n", (uint32_t)partition_number);
        return false;
    }
    pad_lba(image);
    
    // Goto backup boot sector location and write the VBR and FSInfo there
    fseek(image, (partition.starting_lba + vbr.bpb_bk_boot_sect) * lba_size, SEEK_SET);
    if (fwrite(&vbr, 1, sizeof vbr, image) != sizeof vbr)
    {
        printf("Failed to write VBR to backup sector in partition %u\n", (uint32_t)partition_number);
        return false;
    }
    pad_lba(image);

    if (fwrite(&fsinfo, 1, sizeof fsinfo, image) != sizeof fsinfo)
    {
        printf("Could not write File System Info Sector to backup sector in partition %u\n", (uint32_t)partition_number);
        return false;
    }
    pad_lba(image);

    // Initialize FATs
    uint32_t fat_start = partition.starting_lba + reserved_sectors;
    fseek(image, fat_start * lba_size, SEEK_SET);

    // First two FAT entries are reserved
    uint32_t fat_entries[2] = {0x0FFFFFF8, 0x0FFFFFFF};
    // Root directory cluster (cluster 2) is end of chain
    uint32_t root_dir_entry = 0x0FFFFFFF;

    // Write initial FAT entries
    for (int fat = 0; fat < vbr.bpb_num_fats; fat++) {
        fseek(image, (fat_start + fat * fat_size) * lba_size, SEEK_SET);
        // Write reserved entries
        if (fwrite(fat_entries, sizeof(uint32_t), 2, image) != 2) {
            return false;
        }
        // Write root directory entry
        if (fwrite(&root_dir_entry, sizeof(uint32_t), 1, image) != 1) {
            return false;
        }
        
        // Fill rest of FAT with zeros
        uint32_t zero = 0;
        for (uint32_t i = 3; i < total_clusters; i++) {
            if (fwrite(&zero, sizeof(uint32_t), 1, image) != 1) {
                return false;
            }
        }
    }

    // Initialize root directory (empty)
    uint32_t root_dir_sector = fat_start + (2 * fat_size) + 
                              ((root_cluster - 2) * sectors_per_cluster);
    fseek(image, root_dir_sector * lba_size, SEEK_SET);
    uint8_t empty_cluster[bytes_per_clus];
    for (int i = 0; i < bytes_per_clus; ++i)
    {
        empty_cluster[i] = 0;
    }
    if (fwrite(empty_cluster, bytes_per_clus, 1, image) != 1) {
        return false;
    }

    return true;
}
