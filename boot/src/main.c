#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

	// Clear screen to yellow on green
	uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_YELLOW, EFI_GREEN));
	uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

	// Print text red on black
	uefi_call_wrapper(ST->ConOut->SetAttribute, 2, ST->ConOut, EFI_TEXT_ATTR(EFI_RED,EFI_BLACK));
	uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, L"Press any key to shutdown...");

    // Wait until keypress, then return
    EFI_INPUT_KEY key;
    while (uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &key) != EFI_SUCCESS);
	
    // Return success
    return EFI_SUCCESS;
}
