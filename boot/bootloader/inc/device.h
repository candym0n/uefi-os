#ifndef DISK_H
#define DISK_H

#include <efi.h>
#include <efilib.h>
#include <efigpt.h>
#include <bootio.h>

#define MAX_PARTITIONS 128

typedef enum
{
    FS_NONE,
    FS_FAT32,
    FS_EXT4,
} filesystem_t;

typedef struct
{
    CHAR16 name[36];
    filesystem_t format_type;
    UINT64 size;
    UINT64 offset;
    EFI_GUID type_guid;
    EFI_GUID unique_guid;
} partition_info_t;

// Out of all of the devices, find the ONE device that the user chooses to boot from (this system will probably be changed in the future)
EFI_STATUS find_boot_device(OUT EFI_BLOCK_IO_PROTOCOL *block_io);

#endif
