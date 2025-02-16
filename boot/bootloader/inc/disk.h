#ifndef DISK_H
#define DISK_H

#include <efi.h>
#include <efilib.h>
#include <efigpt.h>

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

// Out of all of the disks, find at least one partition of a certain format (allow user to choose if multiple)
EFI_STATUS find_partition_by_format(OUT partition_info_t *partition, filesystem_t format_type);

#endif
