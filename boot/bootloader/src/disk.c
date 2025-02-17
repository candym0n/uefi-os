#include "disk.h"

#define GPT_HEADER_SIGNATURE 0x5452415020494645ULL // "EFI PART"

// Helper function for reading in a number
static const UINTN read_number(const CHAR16 *message, UINTN max)
{
    EFI_STATUS status;

tryagain:
    UINTN selected = 0;
    Print(message);
    while (1)
    {
        EFI_INPUT_KEY key;
        status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2,
                                   ST->ConIn,
                                   &key);

        if (EFI_ERROR(status))
            continue;

        if (key.UnicodeChar >= '0' && key.UnicodeChar <= '9')
        {
            selected = selected * 10 + (key.UnicodeChar - '0');
            Print(L"%c", key.UnicodeChar);
        }
        else if (key.UnicodeChar == 'r')
            goto tryagain;
        else if (key.UnicodeChar == '\r')
            break;
    }

    if (selected >= max)
    {
        Print(L"\nOver");
        goto tryagain;
    }

    Print(L"\n");
    return selected;
}

// Helper function to convert filesystem_t to a string
static const CHAR16 *filesystem_to_string(filesystem_t type)
{
    switch (type)
    {
    case FS_NONE:
        return L"None";
    case FS_FAT32:
        return L"FAT32";
    case FS_EXT4:
        return L"EXT4";
    default:
        return L"Unknown";
    }
}

// Helper function to print all the partitions in a disk
static VOID print_partitions(partition_info_t *partitions, UINTN partition_count)
{
    Print(L"id  | Name                    | Size (MB) | Offset    | GUID\n");
    Print(L"------------------------------------------------------------------------------------------------\n");

    // Print partition information
    for (UINTN i = 0; i < partition_count; ++i)
    {
        Print(L"%3d | %-24s | %8d | %9lld | %g\n",
              i,
              partitions[i].name,
              partitions[i].size / (1024 * 1024),
              partitions[i].offset,
              &partitions[i].unique_guid);
        Print(L"------------------------------------------------------------------------------------------------\n");
    }
}

// Determine which filesystem a partition is formatted to
filesystem_t determine_format_type(partition_info_t partition, EFI_BLOCK_IO_PROTOCOL *block_io)
{
    UINT8 *buffer;
    UINTN buffer_size = 4096; // read first 4KB to check format
    filesystem_t fs = FS_NONE;

    // Allocate buffer for reading parteition data
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, buffer_size, (VOID **)&buffer);

    if (EFI_ERROR(uefi_call_wrapper(block_io->ReadBlocks, 5,
                                    block_io,
                                    block_io->Media->MediaId,
                                    partition.offset / block_io->Media->BlockSize,
                                    buffer_size,
                                    buffer)))
        goto done;

    // Check for FAT32
    if (buffer[82] == 'F' && buffer[83] == 'A' && buffer[84] == 'T' &&
        buffer[85] == '3' && buffer[86] == '2')
    {
        fs = FS_FAT32;
        goto done;
    }

    // Check for EXT4
    else if (buffer[1080] == 0x53 && buffer[1081] == 0xEF)
    {
        fs = FS_EXT4;
        goto done;
    }

done:
    uefi_call_wrapper(BS->FreePool, 1, buffer);
    return fs;
}

EFI_STATUS find_partition_by_format(OUT partition_info_t *partition, filesystem_t format_type)
{
    EFI_STATUS status;
    EFI_HANDLE *handle_buffer;
    UINTN handle_count;
    partition_info_t partitions[MAX_PARTITIONS];
    UINTN partition_count = 0;
    UINTN i;

    // Get all Block IO handles
    status = uefi_call_wrapper(BS->LocateHandleBuffer, 5,
                               ByProtocol,
                               &gEfiBlockIoProtocolGuid,
                               NULL,
                               &handle_count,
                               &handle_buffer);

    if (EFI_ERROR(status))
    {
        Print(L"Failed to locate Block IO handles!\n");
        return status;
    }

    // Iterate through all Block IO devices
    for (i = 0; i < handle_count; ++i)
    {
        EFI_BLOCK_IO_PROTOCOL *block_io;
        EFI_PARTITION_TABLE_HEADER *gpt_header;
        EFI_PARTITION_ENTRY *partition_entry;
        UINTN entry_size;
        UINTN num_entries;

        // Get the Block I/O protocol interface
        status = uefi_call_wrapper(BS->HandleProtocol, 3,
                                   handle_buffer[i],
                                   &gEfiBlockIoProtocolGuid,
                                   (VOID **)&block_io);

        if (EFI_ERROR(status) || block_io->Media->LogicalPartition)
            continue;

        // Allocate space for the GPT header
        uefi_call_wrapper(BS->AllocatePool, 3,
                          EfiLoaderData,
                          block_io->Media->BlockSize,
                          (VOID **)&gpt_header);

        // Read it! (second LBA block)
        uefi_call_wrapper(block_io->ReadBlocks, 5,
                          block_io,
                          block_io->Media->MediaId,
                          1,
                          block_io->Media->BlockSize,
                          gpt_header);

        // Validate the signature
        if (gpt_header->Header.Signature != GPT_HEADER_SIGNATURE)
        {
            uefi_call_wrapper(BS->FreePool, 1, gpt_header);
            continue;
        }

        // Allocate and read partition entries
        entry_size = gpt_header->SizeOfPartitionEntry;
        num_entries = gpt_header->NumberOfPartitionEntries;

        status = uefi_call_wrapper(BS->AllocatePool, 3,
                                   EfiLoaderData,
                                   entry_size * num_entries,
                                   (VOID **)&partition_entry);

        status = uefi_call_wrapper(block_io->ReadBlocks, 5,
                                   block_io,
                                   block_io->Media->MediaId,
                                   gpt_header->PartitionEntryLBA,
                                   entry_size * num_entries,
                                   partition_entry);

        // Process each partition entry
        for (UINTN j = 0; j < num_entries; ++j)
        {
            if (partition_count >= MAX_PARTITIONS)
                break;

            // Check that the partition is not unused
            if (CompareGuid(&partition_entry[j].PartitionTypeGUID, &NullGuid) == 0)
            {
                continue;
            }

            // Fill out the partition information
            partitions[partition_count].size = partition_entry[j].EndingLBA - partition_entry[j].StartingLBA - 1;
            partitions[partition_count].size *= block_io->Media->BlockSize;
            partitions[partition_count].offset = partition_entry[j].StartingLBA * block_io->Media->BlockSize;
            partitions[partition_count].format_type = determine_format_type(partitions[partition_count], block_io);
            CopyMem(&partitions[partition_count].type_guid, &partition_entry[j].PartitionTypeGUID, sizeof(EFI_GUID));
            CopyMem(&partitions[partition_count].unique_guid, &partition_entry[j].UniquePartitionGUID, sizeof(EFI_GUID));
            StrCpy(partitions[partition_count].name, partition_entry[j].PartitionName);

            // Check if we don't have a match
            if (partitions[partition_count].format_type != format_type)
                continue;

            // Update the partition count value
            ++partition_count;
        }

        // Print partition information
        Print(L"EXT4 formatted partitions in disk %d:\n", i);
        print_partitions(partitions, partition_count);

        // Free the GPT Header and entries
        uefi_call_wrapper(BS->FreePool, 1, partition_entry);
        uefi_call_wrapper(BS->FreePool, 1, gpt_header);

        // Check if THIS is the disk we wnat
        if (read_number(L"\nUse this disk? 1 for yes, 0 for no. (0):", 2) == 1)
        {
            uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
            break;
        }
        
        uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
    }

    // Check if this is the last disk
    if (i >= handle_count)
    {
        Print(L"Uh oh, ran out of disks.\n");
        return EFI_ABORTED;
    }

    partition_info_t selected_partition;

    // Check if we don't have any partitions
    if (partition_count == 0)
        return EFI_NOT_FOUND;

    // Check if we only have one partitions
    if (partition_count == 1)
    {
        selected_partition = partitions[0];
        goto done;
    }

    // Print information about partitions
    Print(L"Using disk %u\n", i);
    Print(L"Multiple %s formatted partitions were found:\n\n", filesystem_to_string(format_type));
    print_partitions(partitions, partition_count);

    // Wait for user input
    selected_partition =  partitions[read_number(
        L"\nPlease input the id of the one to boot from or press r to start over again (0): ",
        partition_count
    )];

done:
    *partition = selected_partition;
    return EFI_SUCCESS;
}