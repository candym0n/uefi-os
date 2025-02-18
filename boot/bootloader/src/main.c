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
    status = find_boot_device(&block_io);

    if (EFI_ERROR(status))
        Print(L"Error finding device: %r\n", status);
    else
    {
        uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);
        Print(L"You did it, oh yeah!");
    }

    while (1){}
    return status;
}
