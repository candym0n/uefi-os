#include <bootio.h>

const UINTN read_number(const CHAR16 *message, UINTN max)
{
    EFI_STATUS status;

tryagain:
    UINTN selected = 0;
    INTN typed = 0;
    Print(message);
    while (TRUE)
    {
        EFI_INPUT_KEY key;
        status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2,
            ST->ConIn,
            &key
        );

        if (EFI_ERROR(status))
            continue;

        if (key.UnicodeChar >= '0' && key.UnicodeChar <= '9')
        {
            selected = selected * 10 + (key.UnicodeChar - '0');
            Print(L"%c", key.UnicodeChar);
            ++typed;
        }
        else if (key.UnicodeChar == CHAR_BACKSPACE && typed > 0)
        {
            selected = selected / 10;
            Print(L"\b \b");
            --typed;
        }
        else if (key.UnicodeChar == CHAR_CARRIAGE_RETURN)
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

BOOLEAN yes_or_no(const CHAR16 *message)
{
    EFI_STATUS status;

    BOOLEAN result = FALSE;
    Print(message);
    Print(L"N");
    while (TRUE)
    {
        EFI_INPUT_KEY key;
        status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2,
            ST->ConIn,
            &key
        );
        if (EFI_ERROR(status))
            continue;

        if (key.UnicodeChar == CHAR_CARRIAGE_RETURN)
            break;
        else if (key.UnicodeChar == 'y')
        {
            if (result <= 1)
                Print(L"\b \b");
            Print(L"Y");
            result = 1;
        }
        else if (key.UnicodeChar == 'n')
        {
            if (result <= 1)
                Print(L"\b \b");
            Print(L"N");
            result = 0;
        }
    }

    return result;
}
