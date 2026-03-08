#ifndef CANDYFS_H
#define CANDYFS_H

#include <stdint.h>
#include <common/string.h>

#define BLOCK_SIZE 4096
#define GROUP_SIZE (BLOCK_SIZE * 8) // The number of blocks in a block group

#define INODES_PER_GROUP 8192 // 1 : 16 KiB (with 4 KiB block size and 128 MiB group size)

#define SB_MAGIC 0xCAFE
#define EXTENT_MAGIC 0xBEEF

#define CFS_TREE_ORDER 8              // The order of the B+ tree for the directory structure (hopefully an even number)
#define CFS_TREE_NODE_SIZE BLOCK_SIZE // The size of a B+ tree node should be that of a block
#define CFS_KEY_SIZE 256              // The size of a B+ tree key
#define CFS_NAME_LEN 255              // The maximum length of a file name

#define CFS_SB_NAME_LEN 32 // The length of the volume name (cfs_superblock_t.name)

#define CFS_IT_SPAN ((sizeof(cfs_inode_t) * INODES_PER_GROUP + BLOCK_SIZE - 1) / BLOCK_SIZE) // The number of blocks that an inode table spans

// NOTE: The following are all only for block groups that are not the first
#define BLOCK_BITMAP_BLOCK(group) (group * GROUP_SIZE)
#define INODE_BITMAP_BLOCK(group) (group * GROUP_SIZE + 1)
#define INODE_TABLE_BLOCK(group) (group * GROUP_SIZE + 2)

#define RSVD_INODES 2
#define BAD_BLKS_INODES 1
#define ROOT_INODE 2

#define ROOT_USER_UUID 0
#define ROOT_GROUP_UUID 0

/*
 * File metadata (permissions, types, etc)
 */

// File / directory permissions
typedef enum
{
    READ = 0b001,
    WRITE = 0b010,
    EXECUTE = 0b100
} cfs_permissions_t;

// Flags for the inodes
typedef enum
{
    INODE_INLINE = 0b0001, // The data is stored inline (instead of extents)
} cfs_inode_flags_t;

// Different types of files
typedef enum __attribute__((packed))
{
    UNKNOWN = 0, // IDK why this would ever exist
    REGULAR,     // For your many cat.png's
    DIRECTORY,   // For seperating your cat.png's by breed
    CHARACTER,   // For reading your input from the keyboard for which cat.png to see
    BLOCK,       // For reading cat images off of your disk
    FIFO,        // For letting your cat image viewer talk with your cat image downloader
    SOCKET,      // For browsing catpngs.com
    SYMLINK,     // For linking to your cat.png files
} cfs_file_type_t;

// File mode
typedef struct
{
    cfs_file_type_t type : 4;                 // The type of the file
    cfs_permissions_t owner_permissions : 3;  // The permissions of the owner
    cfs_permissions_t group_permissions : 3;  // The permissions of the group
    cfs_permissions_t others_permissions : 3; // The permissions of others
} __attribute__((packed)) cfs_file_mode_t;

/*
 * General metadata structures (superblock, group descriptor)
 */

// Superblock - resides in the first block of the volume (and is actually the size of the entire block)
typedef struct
{
    uint8_t open[1024]; // First 1024 bytes left open for boot sector or other stuff

    char16_t name[CFS_SB_NAME_LEN]; // The name of the volume
    uint8_t uuid[16];               // Unique identifier for the volume
    uint16_t magic;                 // Magic signature (0xCAFE)

    uint64_t inodes_count; // The number of inodes in the volume
    uint64_t blocks_count; // The number of blocks in the volume
    uint32_t group_count;  // The number of block groups in the volume

    uint64_t free_inodes_count; // The number of free inodes in the volume
    uint64_t free_blocks_count; // The number of free blocks in the volume

    uint32_t block_size;       // The size of a block
    uint32_t block_group_size; // The number of blocks in a block group

    uint32_t reserved_inodes;      // The number of inodes that are reserved
    uint64_t bad_blks_inode_index; // The index of the bad blocks inode
    uint64_t root_inode_index;     // The index of the root directory inode (2, 1 is bad blocks)

    uint32_t inodes_per_group; // The number of inodes in a block group
    uint32_t inode_size;       // The size of an inode in bytes

    uint64_t gdt_start; // The first block of the GDT
    uint16_t gdt_span;  // The number of blocks the GDT spans

    uint64_t creation_time; // The time when the filesystem was created

    uint16_t tree_order; // The order of the B+ tree for the directory structure

    uint8_t padding[BLOCK_SIZE - (
        1024 +              // 1024 bytes left open
        2 * 3 +             // uint16_t's
        4 * 6 +             // uint32_t's
        8 * 8 +             // uint64_t's
        16 +                // 16 byte UUID
        CFS_SB_NAME_LEN * 2 // Name
        ) - 4   // 4 byte CRC32 checksum
    ];          // Reserved for future use

    uint32_t checksum; // CRC32 checksum for the superblock

} __attribute__((packed)) cfs_superblock_t;

// Group Descriptor - describes a block group
typedef struct
{
    uint64_t block_bitmap; // The block containing the block bitmap (0 means uninitialized)
    uint64_t inode_bitmap; // The block containing the inode bitmap (0 means uninitialized)
    uint64_t inode_table;  // The block where the inode table starts (0 means uninitialized)

    uint32_t free_inodes; // The number of free inodes in this block group
    uint32_t free_blocks; // The number of free blocks in this block group
    uint32_t dir_count;   // The number of directories currently in the block group

    uint8_t reserved[24]; // Reserved for future use

    uint32_t checksum; // Checksum for the entry
} __attribute__((packed)) cfs_group_desc_t;

/*
 * Inode structures (extent, inode)
 */

// Extent Header - describes the extent tree
typedef struct
{
    uint16_t magic;    // Magic signature (0xCODE)
    uint16_t entries;  // The number of entries in the extent
    uint16_t depth;    // The depth of the extent tree (how many indirections)
    uint16_t reserved; // Reserved for future use
} __attribute__((packed)) cfs_extent_header_t;

// Extent identifier - points to a block containing an extent tree
typedef struct
{
    uint32_t logical_block;  // Every extent in the extent tree has an offset from this block
    uint64_t physical_block; // The physical block containing the extent tree
    uint32_t reserved;       // Reserved for future use
} __attribute__((packed)) cfs_extent_id_t;

// Extent - describes a contiguous block of data
typedef struct
{
    uint32_t logical_block;  // The first logical block extent covers
    uint16_t count;          // The number of blocks covered by the extent
    uint64_t physical_block; // The first physical block of the extent
    uint16_t reserved;       // Reserved for future use
} __attribute__((packed)) cfs_extent_t;

// Inode - describes a file or directory or something else (?)
typedef struct
{
    cfs_file_mode_t mode; // Types and permissions of the inode
    uint16_t user;        // The user that owns the inode
    uint16_t group;       // The group that it is part of

    uint16_t hard_links_count; // The number of hard links to the inode

    uint64_t creation_time;     // The time the inode was created
    uint64_t modification_time; // The time the inode was last modified
    uint64_t access_time;       // The time the inode's contents were accessed

    uint64_t byte_size;   // The size of the file in bytes
    uint64_t block_count; // The number of blocks the file uses

    uint64_t extents[1 + 2 * 4]; // Header + 4 ids / extents (or just raw data for inline files)

    uint32_t reserved; // Reserved for future use

    uint32_t checksum; // Checksum for the inode
} __attribute__((packed)) cfs_inode_t;

/*
 * Directory structures (entries, B+ trees...)
 * (Note: All 'block addresses' are logical block numbers of the directory file)
 */

// B+ Tree key data structure
typedef struct
{
    char name[CFS_NAME_LEN];                   // The name of the file or directory
    uint8_t zero[CFS_KEY_SIZE - CFS_NAME_LEN]; // Zero padding to make the key size constant
} __attribute__((packed)) cfs_tree_key_t;

// A directory entry
typedef struct
{
    uint64_t next;                             // The next node in the linked list
    uint64_t inode;                            // The inode of the file
    uint8_t reserved[CFS_TREE_ORDER * 8 - 16]; // Reserved for future use
} __attribute__((packed)) dir_entry_t;

// B+ Tree Node - describes a node in the B+ tree
typedef struct
{
    uint16_t n;      // The current number of keys in the node
    uint8_t leaf;    // Whether the node is a leaf
    uint8_t padding; // Padding for alignment
    union
    {
        dir_entry_t entry;                 // The directory entry (if leaf)
        uint64_t children[CFS_TREE_ORDER]; // The blocks of the children of the node (if internal)
    } data;
    cfs_tree_key_t keys[CFS_TREE_ORDER]; // The keys of the node
} __attribute__((packed)) cfs_tree_node_t;

#endif // CANDYFS_H
