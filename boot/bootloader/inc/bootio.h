#ifndef BOOTIO_H
#define BOOTIO_H

#include <efi.h>
#include <efilib.h>

// Read a number from 0 - max (default is 0)
UINTN read_number(const CHAR16 *message, UINTN max);

// Read Y or N (default is N)
BOOLEAN yes_or_no(const CHAR16 *message);

#endif
