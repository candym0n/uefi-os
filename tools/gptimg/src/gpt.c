#include "gpt.h"
#include "helpers.h"

#define EMPTY_TABLE_CRC32 0xAB54D286 // The CRC32 checksum of 128 * 128 0's

// Unused partition
const guid_t UNUSED_GUID = {0};

// EFI System Partition GUID
const guid_t ESP_GUID = {0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};

// (Microsoft) Basic Data GUID
const guid_t BASIC_DATA_GUID = {0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, {0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}};


bool write_mbr(FILE *image, uint64_t image_size_lbas)
{
    // Round down the size of the image in LBAs to 0xFFFFFFFF + 1
    if (image_size_lbas > 0xFFFFFFFF)
        image_size_lbas = 0x100000000;

    // Define the MBR to write
    mbr_t mbr = {
        .boot_code = {0}, // Only executed by legacy BIOS systems
        .mbr_signature = 0,   // Unused by UEFI
        .unknown = 0,         // Unused by UEFI
        .partition[0] = {
            .boot_indicator = 0,                // Not bootable
            .starting_chs = {0x00, 0x02, 0x00}, // The start of the GPT header
            .os_type = 0xEE,                    // Protective GPT
            .ending_chs = {0xFF, 0xFF, 0xFF},   // As big as it gets
            .starting_lba = 0x00000001,         // The start of the GPT header
            .size_lba = image_size_lbas - 1     // The size of the disk minus 1
        },
        .boot_signature = 0xAA55};

    // Write it to disk
    if (fwrite(&mbr, 1, sizeof mbr, image) != sizeof mbr)
    {
        return false;
    }

    // Pad to fill a LBA block
    pad_lba(image);

    return true;
}

bool write_gpt_headers(FILE *image, uint64_t image_size_lbas)
{
    // Fill out primary GPT header
    uint32_t gpt_table_lbas = GPT_TABLE_SIZE / lba_size;
    gpt_header_t gpt_header = {
        .signature = {"EFI PART"},
        .revision = 0x00010000, // Version 1.0
        .header_size = 92,      // All headers are this size
        .header_crc32 = 0,      // Will calculate later
        .reserved1 = 0,
        .my_lba = 1,                                                 // LBA 1 is right after MBR
        .alternate_lba = image_size_lbas - 1,                        // Block right before end of file
        .first_usable_lba = 1 + 1 + gpt_table_lbas,                  // MBR + GPT Header + GPT Table
        .last_usable_lba = image_size_lbas - 1 - gpt_table_lbas - 1, // Image size minus secondary GPT Header and Table
        .disk_guid = random_guid(),                                  // It does not matter what is here
        .partition_table_lba = GPT_TABLE_START,                      // After MBR + GPT header
        .number_of_entries = GPT_TABLE_ENTRY_COUNT,
        .size_of_entry = GPT_TABLE_ENTRY_SIZE,
        .partition_table_crc32 = 0, // Will calculate later
        .reserved2 = {0}};

    // Fill out primary header CRC32 values
    gpt_header.partition_table_crc32 = EMPTY_TABLE_CRC32;
    gpt_header.header_crc32 = calculate_crc32(&gpt_header, gpt_header.header_size);

    // Write primary GPT Header to file
    if (fwrite(&gpt_header, sizeof gpt_header, 1, image) != 1)
    {
        printf("Failed to write Primary GPT Header.\n");
        return false;
    }
    pad_lba(image);

    // Fill out secondary GPT header
    gpt_header.alternate_lba = gpt_header.my_lba;                          // Switch the alternate and primary GPT's
    gpt_header.my_lba = image_size_lbas - 1;                               // Header is at the end of the disk image
    gpt_header.partition_table_lba = image_size_lbas - 1 - gpt_table_lbas; // Image size minus secondary GPT Header and Table

    // Fill out secondary header CRC32 values
    gpt_header.header_crc32 = 0;
    gpt_header.header_crc32 = calculate_crc32(&gpt_header, gpt_header.header_size);

    // Go to position of secondary table
    fseek(image, gpt_header.my_lba * lba_size, SEEK_SET);

    // Write secondary GPT table to file
    if (fwrite(&gpt_header, sizeof gpt_header, 1, image) != 1)
    {
        printf("Failed to write Secondary GPT Header.\n");
        return false;
    }

    // Pad it to an LBA block
    pad_lba(image);

    return true;
}

bool add_gpt_partition(FILE *image, uint64_t size, guid_t guid, char16_t *name)
{
    gpt_header_t primary_header;
    gpt_partition_entry_t new_partition = {0};

    // Read primary GPT header (LBA 1)
    fseek(image, lba_size, SEEK_SET);
    if (fread(&primary_header, sizeof(primary_header), 1, image) != 1) {
        printf("Failed to read GPT header!\n");
        return false;
    }

    // Validate GPT signature (in case this is a corrupt image)
    if (memcmp(primary_header.signature, "EFI PART", 8) != 0) {
        printf("Invalid signature in GPT Header!\n");
        return false;
    }
    
    // Read the partition table
    fseek(image, GPT_TABLE_START * lba_size, SEEK_SET);
    gpt_partition_entry_t *partition_table = malloc(GPT_TABLE_SIZE);
    if (!partition_table) {
        printf("Failed to allocate memory for partition table!\n");
        return false;
    }

    if (fread(partition_table, GPT_TABLE_SIZE, 1, image) != 1) {
        printf("Failed to read the partition table!\n");
        free(partition_table);
        return false;
    }

    // Get the last partition entry in the table
    uint64_t last_used_lba = primary_header.first_usable_lba;
    uint32_t partition_count = 0;

    for (uint32_t i = 0; i < GPT_TABLE_ENTRY_COUNT; i++) {
        if (memcmp(&partition_table[i].partition_type_guid, &(guid_t){0}, sizeof(guid_t)) != 0) {
            last_used_lba = partition_table[i].ending_lba;
            partition_count++;
        }
        else break;
    }

    // Check if we have space in the partition table
    if (partition_count >= GPT_TABLE_ENTRY_COUNT) {
        free(partition_table);
        printf("No free partition entries available!\n");
        return false;
    }

    // Calculate new partition boundaries
    uint64_t new_start_lba = next_aligned_lba(last_used_lba) + 1;
    uint64_t new_end_lba = next_aligned_lba(new_start_lba + (size));

    // Verify we don't exceed the last usable LBA
    if (new_end_lba > primary_header.last_usable_lba) {
        free(partition_table);
        printf("Out of space! %llu sectors over.\n", new_end_lba - primary_header.last_usable_lba);
        return false;
    }

    // Prepare new partition entry
    new_partition.partition_type_guid = guid;
    new_partition.unique_partition_guid = random_guid();
    new_partition.starting_lba = new_start_lba;
    new_partition.ending_lba = new_end_lba;
    new_partition.attributes = 0;
    memcpy(new_partition.name, name, 72); // 36 char16_t = 72 bytes

    // Add new partition to the table in memory
    memcpy(&partition_table[partition_count], &new_partition, sizeof(gpt_partition_entry_t));
    
    // Update primary GPT
    // Calculate new CRC32 for partition table
    primary_header.partition_table_crc32 = calculate_crc32(partition_table, GPT_TABLE_SIZE);
    
    // Calculate new header CRC32
    uint32_t saved_crc32 = primary_header.header_crc32;
    primary_header.header_crc32 = 0;
    primary_header.header_crc32 = calculate_crc32(&primary_header, primary_header.header_size);

    // Write updated partition table (primary)
    fseek(image, GPT_TABLE_START * lba_size, SEEK_SET);
    if (fwrite(partition_table, GPT_TABLE_SIZE, 1, image) != 1) {
        free(partition_table);
        printf("Failed to write updated partition table!\n");
        return false;
    }

    // Write updated primary header
    fseek(image, lba_size, SEEK_SET);
    if (fwrite(&primary_header, sizeof(primary_header), 1, image) != 1) {
        free(partition_table);
        printf("Failed to write updated primary GPT header!\n");
        return false;
    }

    // Update secondary GPT
    // Prepare secondary header (copy of primary with adjusted values)
    gpt_header_t secondary_header = primary_header;
    secondary_header.my_lba = primary_header.alternate_lba;
    secondary_header.alternate_lba = primary_header.my_lba;
    secondary_header.partition_table_lba = primary_header.alternate_lba - 
        ((GPT_TABLE_SIZE + lba_size - 1) / lba_size);
    secondary_header.header_crc32 = 0;
    secondary_header.header_crc32 = calculate_crc32(&secondary_header, secondary_header.header_size);

    // Write secondary partition table
    fseek(image, secondary_header.partition_table_lba * lba_size, SEEK_SET);
    if (fwrite(partition_table, GPT_TABLE_SIZE, 1, image) != 1) {
        free(partition_table);
        printf("Failed to write secondary partition table!\n");
        return false;
    }

    // Write secondary header
    fseek(image, secondary_header.my_lba * lba_size, SEEK_SET);
    if (fwrite(&secondary_header, sizeof(secondary_header), 1, image) != 1) {
        free(partition_table);
        printf("Failed to write secondary GPT header!\n");
        return false;
    }

    printf("%u", partition_count + 1);  // +1 since most tools start at partition 1

    // Cleanup
    free(partition_table);
    return true;
}

guid_t get_guid(char *type)
{
    if (strcmp(type, "efi") == 0)
    {
        return ESP_GUID;
    }
    else if (strcmp(type, "basic-data") == 0)
        return BASIC_DATA_GUID;

    printf("Could not find GUID for type %s.\n", type);
    return UNUSED_GUID;
}


