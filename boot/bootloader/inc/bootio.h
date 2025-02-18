#ifndef BOOTIO_H
#define BOOTIO_H

#include <efi.h>
#include <efilib.h>

const UINTN read_number(const CHAR16 *message, UINTN max);

#endif
