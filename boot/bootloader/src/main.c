#include <efi.h>
#include <efilib.h>
#include <efigpt.h>
#include <device.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    
    InitializeLib(ImageHandle, SystemTable);

    // Clear the screen
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    // Find the partition to boot to
    EFI_BLOCK_IO_PROTOCOL block_io;
    partition_info_t partition;
    status = find_boot_partition(&block_io, &partition);

    if (EFI_ERROR(status))
        Print(L"Error finding partition: %r\n", status);
    else
    {
        uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
        Print(L"Chose partition %s.\n", partition.name);
    }

    while (1){}
    return status;
}
