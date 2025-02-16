#include <efi.h>
#include <efilib.h>
#include <efigpt.h>

#define MAX_PARTITIONS 128

#define GPT_HEADER_SIGNATURE 0x5452415020494645ULL // "EFI PART"

typedef struct {
    CHAR16 Name[36];
    CHAR16 FormatType[16];
    UINT64 Size;
    UINT64 Offset;
    EFI_GUID TypeGUID;
} PARTITION_INFO;

// Function to determine partition format type
VOID DetermineFormatType(PARTITION_INFO *Part, EFI_BLOCK_IO_PROTOCOL *BlockIo) {
    UINT8 *Buffer;
    UINTN BufferSize = 4096;  // Read first 4KB to check format
    
    // Allocate buffer for reading partition data
    uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, BufferSize, (VOID**)&Buffer);
    
    if (EFI_ERROR(uefi_call_wrapper(BlockIo->ReadBlocks, 5, BlockIo, 
        BlockIo->Media->MediaId, Part->Offset / BlockIo->Media->BlockSize, 
        BufferSize, Buffer))) {
        StrCpy(Part->FormatType, L"Unformatted");
        uefi_call_wrapper(BS->FreePool, 1, Buffer);
        return;
    }
    
    // Check for FAT32
    if (Buffer[82] == 'F' && Buffer[83] == 'A' && Buffer[84] == 'T' && 
        Buffer[85] == '3' && Buffer[86] == '2') {
        StrCpy(Part->FormatType, L"FAT32");
    }
    // Check for EXT4 (look for magic number and superblock signature)
    else if (Buffer[1080] == 0x53 && Buffer[1081] == 0xEF) {
        StrCpy(Part->FormatType, L"EXT4");
    }
    else {
        StrCpy(Part->FormatType, L"Unformatted");
    }
    
    uefi_call_wrapper(BS->FreePool, 1, Buffer);
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_HANDLE *HandleBuffer;
    UINTN HandleCount;
    PARTITION_INFO Partitions[MAX_PARTITIONS];
    UINTN PartitionCount = 0;
    UINTN Index;
    UINTN Selected;
    
    InitializeLib(ImageHandle, SystemTable);
    
    // Get all Block IO handles
    Status = uefi_call_wrapper(BS->LocateHandleBuffer, 5,
        ByProtocol,
        &gEfiBlockIoProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Failed to locate Block IO handles\n");
        return Status;
    }
    
    // Iterate through all Block IO devices
    for (Index = 0; Index < HandleCount; Index++) {
        EFI_BLOCK_IO_PROTOCOL *BlockIo;
        EFI_PARTITION_TABLE_HEADER *GptHeader;
        EFI_PARTITION_ENTRY *PartEntry;
        UINTN EntrySize;
        UINTN NumEntries;
        
        Status = uefi_call_wrapper(BS->HandleProtocol, 3,
            HandleBuffer[Index],
            &gEfiBlockIoProtocolGuid,
            (VOID**)&BlockIo
        );
        
        if (EFI_ERROR(Status) || BlockIo->Media->LogicalPartition) {
            continue;
        }
        
        // Allocate and read GPT header
        uefi_call_wrapper(BS->AllocatePool, 3, 
            EfiLoaderData, 
            BlockIo->Media->BlockSize, 
            (VOID**)&GptHeader
        );
        
        uefi_call_wrapper(BlockIo->ReadBlocks, 5,
            BlockIo,
            BlockIo->Media->MediaId,
            1,
            BlockIo->Media->BlockSize,
            GptHeader
        );
        
        if (GptHeader->Header.Signature != GPT_HEADER_SIGNATURE) {
            uefi_call_wrapper(BS->FreePool, 1, GptHeader);
            continue;
        }
        
        // Read partition entries
        EntrySize = GptHeader->SizeOfPartitionEntry;
        NumEntries = GptHeader->NumberOfPartitionEntries;
        
        uefi_call_wrapper(BS->AllocatePool, 3,
            EfiLoaderData,
            EntrySize * NumEntries,
            (VOID**)&PartEntry
        );
        
        uefi_call_wrapper(BlockIo->ReadBlocks, 5,
            BlockIo,
            BlockIo->Media->MediaId,
            GptHeader->PartitionEntryLBA,
            EntrySize * NumEntries,
            PartEntry
        );
        
        // Process each partition entry
        for (UINTN i = 0; i < NumEntries; i++) {
            if (PartitionCount >= MAX_PARTITIONS) break;
            
            if (CompareGuid(&PartEntry[i].PartitionTypeGUID, NULL) == 0) {
                continue;
            }
            
            // Store partition information
            Partitions[PartitionCount].Size = PartEntry[i].EndingLBA - PartEntry[i].StartingLBA + 1;
            Partitions[PartitionCount].Size *= BlockIo->Media->BlockSize;
            Partitions[PartitionCount].Offset = PartEntry[i].StartingLBA * BlockIo->Media->BlockSize;
            CopyMem(&Partitions[PartitionCount].TypeGUID, &PartEntry[i].PartitionTypeGUID, sizeof(EFI_GUID));
            
            // Convert partition name from UTF-16 to ASCII
            for (UINTN j = 0; j < 36 && PartEntry[i].PartitionName[j]; j++) {
                Partitions[PartitionCount].Name[j] = PartEntry[i].PartitionName[j];
            }
            
            DetermineFormatType(&Partitions[PartitionCount], BlockIo);
            PartitionCount++;
        }
        
        uefi_call_wrapper(BS->FreePool, 1, PartEntry);
        uefi_call_wrapper(BS->FreePool, 1, GptHeader);
    }
    
    // Display partition information
    Print(L"\nAvailable Partitions:\n");
    Print(L"--------------------------------------------------------------------------------\n");
    Print(L"idx | Name                    | Format      | Size (MB) | Offset    | GUID\n");
    Print(L"--------------------------------------------------------------------------------\n");
    
    for (Index = 0; Index < PartitionCount; Index++) {
        Print(L"%3d | %-24s | %-11s | %8d | %9lld | %g\n",
            Index,
            Partitions[Index].Name,
            Partitions[Index].FormatType,
            Partitions[Index].Size / (1024 * 1024),
            Partitions[Index].Offset,
            &Partitions[Index].TypeGUID
        );
    }
    
    Print(L"--------------------------------------------------------------------------------\n");
    Print(L"Select partition (0-%d): ", PartitionCount - 1);
    
    // Get user input
    Selected = 0;
    while (1) {
        EFI_INPUT_KEY Key;
        Status = uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2,
            SystemTable->ConIn,
            &Key
        );
        
        if (EFI_ERROR(Status)) {
            continue;
        }
        
        if (Key.UnicodeChar >= '0' && Key.UnicodeChar <= '9') {
            UINTN NewSelected = Selected * 10 + (Key.UnicodeChar - '0');
            if (NewSelected < PartitionCount) {
                Selected = NewSelected;
            }
        }
        else if (Key.ScanCode == SCAN_F1) {
            break;
        }
    }
    
    Print(L"\nSelected partition %d: %s\n", Selected, Partitions[Selected].Name);
    
    uefi_call_wrapper(BS->FreePool, 1, HandleBuffer);
    return EFI_SUCCESS;
}