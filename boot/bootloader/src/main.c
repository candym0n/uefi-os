#include <efi.h>
#include <efilib.h>
#include <efigpt.h>
#include <disk.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    
    InitializeLib(ImageHandle, SystemTable);

    // Clear the screen
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    // Find the partition to boot to
    partition_info_t boot_partition;
    status = find_partition_by_format(&boot_partition, FS_EXT4);

    if (EFI_ERROR(status))
        Print(L"Error finding partition: %r\n", status);
    else
        Print(L"Selected partition %s\n", boot_partition.name);

    while (1){}
    return status;
}
