#ifndef GPT_H
#define GPT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <common/crc32.h>
#include "config.h"

// --------------------------
// Magnificent Macros
// --------------------------

// Round up to the nearest alignment value
#define next_aligned_lba(lba) ((lba) - ((lba) % ALIGN_LBA) + ALIGN_LBA - 1)
#define GPT_TABLE_START 2   // After MBR and primary GPT header
#define GPT_TABLE_ENTRY_SIZE 128
#define GPT_TABLE_ENTRY_COUNT 128
#define GPT_TABLE_SIZE (GPT_TABLE_ENTRY_COUNT * GPT_TABLE_ENTRY_SIZE)

// --------------------------
// Terrific Typedefs
// --------------------------

// MBR partition
typedef struct
{
    uint8_t boot_indicator;  // 0x80 for bootable legacy partition, unused by UEFI
    uint8_t starting_chs[3]; // Start of partition in CHS address format, unused by UEFI
    uint8_t os_type;         // The type of partition
    uint8_t ending_chs[3];   // End of partition in CHS address format, unsed by UEFI
    uint32_t starting_lba;   // Starting LBA of the partition, used by UEFI
    uint32_t size_lba;       // The size of the partition in logical block units
} __attribute__((packed)) mbr_partition_t;

// Matser Boot Record
typedef struct
{
    uint8_t boot_code[440];         // Unused by UEFI systems, entry point for legacy BIOS systems, configuration for us :)
    uint32_t mbr_signature;         // Unused. Set to zero.
    uint16_t unknown;               // Unused. Set to zero.
    mbr_partition_t partition[4];   // Array of 4 legacy MBR partition records
    uint16_t boot_signature;        // Set to 0xAA55 to indicate a bootable sector (for legacy BIOS systems)
} __attribute__((packed)) mbr_t;

// Globally Unique IDentifier (GUID)
typedef struct
{
    uint32_t time_lo;               // Lowest 4 bytes of time
    uint16_t time_mid;              // Upper 2 bytes of time
    uint16_t time_hi_and_ver;       // Highest 4 bits are version number
    uint8_t clock_seq_hi_and_res;   // Highest 4 bits are variant number
    uint8_t clock_seq_lo;           // Lowest 8 bits of clock sequence
    uint8_t node[6];                // IEEE 802 address from network card if available. Or just good ol' random numbers.
} __attribute__((packed)) guid_t;

// GPT Header
typedef struct
{
    uint8_t signature[8];                   // Identifies EFI-compatible partition table header, must be ASCII "EFI PART"
    uint32_t revision;                      // GPT Version. Must be 0x0001 0000 for version 1.0
    uint32_t header_size;                   // Header size in bytes (92 bytes)
    uint32_t header_crc32;                  // CRC32 checksum for the header structure (when this field is zero)
    uint32_t reserved1;                     // Must be zero
    uint64_t my_lba;                        // The LBA that contains this data structure
    uint64_t alternate_lba;                 // LBA address of the alternate GPT Header
    uint64_t first_usable_lba;              // The first usable logical block that may be used by a partition described by a GPT Partition Entry
    uint64_t last_usable_lba;               // The last usable logical block that may be used by a partition described by a GPT  Partition Entry
    guid_t   disk_guid;                     // GUID that can be used to uniquely identify the disk
    uint64_t partition_table_lba;           // The starting LBA of the GPT Partition array
    uint32_t number_of_entries;             // The number of Partition Entries in the GUID Partition Entry array
    uint32_t size_of_entry;                 // The size in bytes of each GPT partition entry
    uint32_t partition_table_crc32;         // The CRC32 of the GPT partition entry array
    uint8_t reserved2[INTERNAL_UNIT - 92];  // Reserved by UEFI and must be zero
} __attribute__((packed)) gpt_header_t;

// GPT Partition Entry
typedef struct {
    guid_t partition_type_guid;             // Defines the partition's purpose
    guid_t unique_partition_guid;           // Unique for every partition entry
    uint64_t starting_lba;                  // Starting LBA of the partition
    uint64_t ending_lba;                    // Ending LBA of the partition
    uint64_t attributes;                    // Attributes about the partition
    char16_t name[36];                      // A human readable name of the partition
} __attribute__((packed)) gpt_partition_entry_t;

// --------------------------
// Fabulous Functions
// --------------------------

// Write the Protective Master Boot Record to a disk image
bool write_mbr(FILE *image, uint64_t image_size_lbas);

// Write GPT headers (primary and secondary)
bool write_gpt_headers(FILE *image, uint64_t image_size_lbas);

// Add a GPT partition
bool add_gpt_partition(FILE *image, uint64_t size, guid_t guid, char16_t *name);

// Get the GUID given a type
guid_t get_guid(char *type);

#endif
