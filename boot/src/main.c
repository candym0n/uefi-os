#include <efi.h>
#include <efilib.h>
#include <common/crc32.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;

    InitializeLib(ImageHandle, SystemTable);

    const char *data = "HELLO";
    Print(L"HERE IT IS: %x", crc32_calculate((void *)data, (size_t)5));
    while (1)
    {
    }
    return status;
}
