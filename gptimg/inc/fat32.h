#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "gpt.h"
#include "helpers.h"

// --------------------
// Terrific Typedefs
// --------------------

// FAT32 Directory Entry Attributes
typedef enum __attribute__((packed))
{
    ATTR_READ_ONLY = 0x01,
    ATTR_HIDDEN = 0x02,
    ATTR_SYSTEM = 0x04,
    ATTR_VOLUME_ID = 0x08,
    ATTR_DIRECTORY = 0x10,
    ATTR_ARCHIVE = 0x20,
    ATTR_LONG_NAME = ATTR_READ_ONLY | ATTR_HIDDEN |
                     ATTR_SYSTEM | ATTR_VOLUME_ID,
} fat32_dir_attr_t;

// FAT32 Volume Boot Record (VBR)
// Contains Boot Sector (BS) code and Bios Parameter Block (BPB)
typedef struct
{
    uint8_t bs_jmp_boot[3];     // Machine code to jump to the entry point (useless)
    uint8_t bs_oem_name[8];     // OEM identifier
    uint16_t bpb_bytes_per_sec; // The number of bytes per sector
    uint8_t bpb_secs_per_clus;  // The number of sectors per cluster
    uint16_t bpb_rsvd_sec_cnt;  // The number of reserved sectors
    uint8_t bpb_num_fats;       // The number of File Allocation Tables
    uint16_t bpb_root_ent_cnt;  // Number of root directory entries
    uint16_t bpb_tot_sec16;     // The total sectors in the logical volume (0 means it is stored in a later field)
    uint8_t bpb_media;          // The media descriptor type
    uint16_t bpb_fatsz16;       // The number of sectors in a FAT (only for FAT 12/16, for 32 this should be 0)
    uint16_t bpb_sec_per_trk;   // The number of sectors per track
    uint16_t bpb_num_heads;     // The number of heads or sides on the storage media
    uint32_t bpb_hidd_sec_cnt;  // The number of hidden sectors (i.e the LBA of the beginning of the partition)
    uint32_t bpb_tot_sec32;     // This is the field for larger logical volumes
    uint32_t bpb_fatsz32;       // The number of sectors per fat for FAT32
    uint16_t bpb_flags;         // Extra flags for FAT32
    uint16_t bpb_fsver;         // The FAT version number (high byte is major version, low byte is minor version
    uint32_t bpb_root_clus;     // The cluster number of the root directory (usually 2)
    uint16_t bpb_fsinfo;        // The sector number of the FSInfo structure
    uint16_t bpb_bk_boot_sect;  // The sector number of the backup boot sector
    uint8_t bpb_reserved[12];   // Reserved, should be 0
    uint8_t bs_drive_num;       // Drive number, identical to result from BIOS interrupt 0x13
    uint8_t bs_reserved;        // Flags in Windows NT. Reserved otherwise.
    uint8_t bs_boot_sig;        // Signature (must be 0x28 or 0x29)
    uint8_t bs_vol_id[4];       // Volume ID 'Serial' number, used for tracking volumes between computers. Can be ignored
    uint8_t bs_vol_lab[11];     // Volume label string. Padded with spaces
    uint8_t bs_fil_sys_type[8]; // System identifier string, always "FAT32 "
    uint8_t boot_code[420];     // Boot code
    uint16_t bootsect_sig;      // Must be 0xAA55
} __attribute__((packed)) vbr_t;

// FAT32 File System Info Sector
typedef struct
{
    uint32_t lead_sig;      // Lead signature (must be 0x41615252)
    uint8_t reserved1[480]; // Reserved bytes, should never be used
    uint32_t struct_sig;    // Another signature (must be 0x61417272)
    uint32_t free_count;    // Contains last known free cluster count on the volume (if 0xFFFFFFFF)
    uint32_t nxt_free;      // Indicates the cluster number at which the file system should start looking for available clusters
    uint8_t reserved2[12];  // More reserved bytes
    uint32_t trail_sig;     // Trail signature (0xAA550000)
} __attribute__((packed)) fs_info_t;

// FAT32 Directory Entry (Short Name)
typedef struct
{
    uint8_t name[11];       // First 8 characters are the name and the last 3 are the extension
    fat32_dir_attr_t attr;  // Attributes of the file
    uint8_t reserved;       // Reserved for use only for Windows NT
    uint8_t crt_time_tenth; // The time of creation in tenths of a second
    uint16_t crt_time;      // The time that the file was created (0bHHHHHMMMMMMSSSSS where seconds are multiplied by two)
    uint16_t crt_date;      // The date that the file was created (0bYYYYYYYMMMMDDDDD)
    uint16_t lst_acc_date;  // Last accessed date (same format as creation)
    uint16_t fst_clus_hi;   // The high 16 bits of the first cluster number
    uint16_t wrt_time;      // Last modification time (same format)
    uint16_t wrt_date;      // Last modification date (same format)
    uint16_t fst_clus_lo;   // The low 16 bits of the first cluster number
    uint32_t file_size;     // The size of the file in bytes
} __attribute((packed)) fat32_dir_entry_short;

// FAT32 File types
typedef enum
{
    TYPE_DIR, // Directory (File Folder)
    TYPE_FILE // Regular file
} file_type_t;

// --------------------
// Fabulous Functions
// --------------------

// Format a partition to FAT32
bool format_partition_to_fat32(FILE *image, uint8_t partition_number, uint32_t bytes_per_clus);

// Create a new directory
bool fat32_create_directory(FILE *image, const char *path, uint32_t parent_cluster);

// Add a file to the filesystem
bool fat32_add_file(FILE *image, const char *filename, FILE *source_file, uint32_t parent_cluster);

// Get a cluster number given a path
uint32_t fat32_get_path_cluster(FILE *image, const char *path);

#endif
