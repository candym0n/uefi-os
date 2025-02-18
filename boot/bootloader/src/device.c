#include <device.h>

#define GPT_HEADER_SIGNATURE 0x5452415020494645ULL // "EFI PART"

// Helper function to convert filesystem_t to a string
static const CHAR16 *filesystem_to_string(filesystem_t type)
{
    switch (type)
    {
    case FS_FAT32:
        return L"FAT32";
    case FS_EXT4:
        return L"EXT4";
    case FS_NONE:
    default:
        return L"Unknown";
    }
}

// Helper function to print all the partitions in a device
static VOID print_partitions(partition_info_t *partitions, UINTN partition_count)
{
    Print(L"id  | Name                    | Size (MB) | Format FS | GUID\n");
    Print(L"------------------------------------------------------------------------------------------------\n");

    // Print partition information
    for (UINTN i = 0; i < partition_count; ++i)
    {
        Print(L"%3d | %-24s | %8d | %-8s | %g\n",
              i,
              partitions[i].name,
              partitions[i].size / (1024 * 1024),
              filesystem_to_string(partitions[i].format_type),
              &partitions[i].unique_guid);
        Print(L"------------------------------------------------------------------------------------------------\n");
    }
}

// Helper function to determine which filesystem a partition is formatted to
static filesystem_t determine_format_type(partition_info_t partition, EFI_BLOCK_IO_PROTOCOL *block_io)
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

// Helper function to find all the partitions in a device (false means error and message should be printed)
static BOOLEAN get_partitions(OUT partition_info_t partitions[MAX_PARTITIONS], OUT UINTN *count, IN EFI_BLOCK_IO_PROTOCOL *block_io)
{
    EFI_STATUS status;
    EFI_PARTITION_TABLE_HEADER *gpt_header;
    EFI_PARTITION_ENTRY *partition_entries;
    UINTN entry_size;
    UINTN num_entries;

    // Read the GPT header
    gpt_header = AllocatePool(block_io->Media->BlockSize);
    status = uefi_call_wrapper(block_io->ReadBlocks, 5,
        block_io,
        block_io->Media->MediaId,
        1,
        block_io->Media->BlockSize,
        gpt_header
    );
    if (EFI_ERROR(status))
    {
        uefi_call_wrapper(BS->FreePool, 1, gpt_header);
        Print(L"Failed to read GPT header! Status: %r\n", status);
        return FALSE;
    }

    // Validate the signature
    if (gpt_header->Header.Signature != GPT_HEADER_SIGNATURE)
    {
        uefi_call_wrapper(BS->FreePool, 1, gpt_header);
        Print(L"Disk is not GPT-Partitioned! (Signature is incorrect)\n");
        return FALSE;
    }

    // Read partition entries
    entry_size = gpt_header->SizeOfPartitionEntry;
    num_entries = gpt_header->NumberOfPartitionEntries;

    partition_entries = AllocatePool(entry_size * num_entries);
    status = uefi_call_wrapper(block_io->ReadBlocks, 5,
        block_io,
        block_io->Media->MediaId,
        gpt_header->PartitionEntryLBA,
        entry_size * num_entries,
        partition_entries
    );

    // Process each partition entry
    UINTN valid_partitions_count = 0;
    for (UINTN j = 0; j < num_entries; ++j)
    {
        // Check that we are not over
        if (valid_partitions_count >= MAX_PARTITIONS)
            break;
        
        // Check that the partition is not unused
        if (CompareGuid(&partition_entries[j].PartitionTypeGUID, &NullGuid) == 0)
            continue;
        
        // Fill out partition information
        partitions[valid_partitions_count].size = partition_entries[j].EndingLBA - partition_entries[j].StartingLBA - 1;
        partitions[valid_partitions_count].size *= block_io->Media->BlockSize;
        partitions[valid_partitions_count].offset = partition_entries[j].StartingLBA * block_io->Media->BlockSize;
        partitions[valid_partitions_count].format_type = determine_format_type(partitions[valid_partitions_count], block_io);
        CopyMem(&partitions[valid_partitions_count].type_guid, &partition_entries[j].PartitionTypeGUID, sizeof(EFI_GUID));
        CopyMem(&partitions[valid_partitions_count].unique_guid, &partition_entries[j].UniquePartitionGUID, sizeof(EFI_GUID));
        StrCpy(partitions[valid_partitions_count].name, partition_entries[j].PartitionName);

        // Update the valid partitions count value
        ++valid_partitions_count;
    }

    // Set the count
    *count = valid_partitions_count;

    return TRUE;
}

EFI_STATUS find_boot_device(OUT EFI_BLOCK_IO_PROTOCOL *block_io)
{
    EFI_STATUS status;
    EFI_HANDLE *handle_buffer;
    UINTN handle_count, selected_device, partition_count;
    partition_info_t partitions[MAX_PARTITIONS];

    // Get all Block IO handles
    status = uefi_call_wrapper(BS->LocateHandleBuffer, 5,
        ByProtocol,
        &gEfiBlockIoProtocolGuid,
        NULL,
        &handle_count,
        &handle_buffer);
    
    if (EFI_ERROR(status))
    {
        Print(L"Failed to locate Block IO handles! Status: %r\n", status);
        return status;
    }

    // Get all of the Block IO devices
    EFI_BLOCK_IO_PROTOCOL *devices[handle_count];
    for (UINTN i = 0; i < handle_count; ++i)
    {
        status = uefi_call_wrapper(BS->HandleProtocol, 3,
            handle_buffer[i],
            &gEfiBlockIoProtocolGuid,
            (VOID**)&devices[i]
        );
        if (EFI_ERROR(status))
        {
            Print(L"Failed to get block io device for handle %d! Status: %r\n", i, status);
            return status;
        }

        // Check if the device's media is present
        if (devices[i]->Media->MediaPresent)
        {
            Print(L"Failed to block io device for handle %d! (Media not present)\n", i);
        }
    }

    // Ask
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
    while (TRUE)
    {
        Print(L"%d handles were found.", handle_count);
        selected_device = read_number(
            L"\nPlease select one device to view the metadata of (0):",
            handle_count
        );

        // Clear the screen
        uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

        // Print the partitions in that device
        if (get_partitions(partitions, &partition_count, devices[selected_device]))
        {
            Print(L"Metadata for device %d\n", selected_device);
            Print(L"Bytes per block: %lu\n", devices[selected_device]->Media->BlockSize);
            Print(devices[selected_device]->Media->RemovableMedia ? L"Removable\n" : L"Nonremovable\n");
            Print(L"Partitions:\n");
            print_partitions(partitions, partition_count);
        }
        else
            continue;
        
        // Check if the user wants to boot from here
        if (yes_or_no(L"\nIs this the device YOU want to use: "))
        {
            *block_io = *devices[selected_device];
            return EFI_SUCCESS;
        }

        Print(L"\n");
    }
}
