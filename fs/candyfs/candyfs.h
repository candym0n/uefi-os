#ifndef CANDYFS_H
#define CANDYFS_H

#include <common/types.h>

#define BLOCK_SIZE 4096
#define GROUP_SIZE BLOCK_SIZE * 8 // The number of blocks in a block group

#define SB_MAGIC 0xCAFE
#define EXTENT_MAGIC 0xCODE

#define CFS_TREE_ORDER 8              // The order of the B+ tree for the directory structure (hopefully an even number)
#define CFS_TREE_NODE_SIZE BLOCK_SIZE // The size of a B+ tree node should be that of a block
#define CFS_KEY_SIZE 256              // The size of a B+ tree key
#define CFS_NAME_LEN 255              // The maximum length of a file name

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
    uint8_t jump[3]; // First three bytes are left open for a jump instruction (if using MBR partitioning)

    char16_t name[16]; // The name of the volume
    uint8_t uuid[16];  // Unique identifier for the volume
    uint16_t magic;    // Magic signature (0xCAFE)

    uint64_t inodes_count; // The number of inodes in the volume
    uint64_t blocks_count; // The number of blocks in the volume

    uint64_t free_inodes_count; // The number of free inodes in the volume
    uint64_t free_blocks_count; // The number of free blocks in the volume

    uint32_t block_size;       // The size of a block
    uint32_t block_group_size; // The number of blocks in a block group

    uint64_t first_usable_block; // The first data block usable
    uint64_t root_inode_index;   // The index of the root directory inode

    uint32_t inodes_per_group; // The number of inodes in a block group
    uint32_t inode_size;       // The size of an inode in bytes

    uint16_t gdt_size; // The number of blocks the GDT spans

    uint64_t creation_time; // The time when the filesystem was created

    uint16_t tree_order; // The order of the B+ tree for the directory structure

    /*
     * The last 4 bytes of the superblock are a
     * checksum for the superblock (in little endian)
     */
} __attribute__((packed)) cfs_superblock_t;

// Group Descriptor - describes a block group
typedef struct
{
    uint64_t block_bitmap; // The block containing the block bitmap
    uint64_t inode_bitmap; // The block containing the inode bitmap
    uint64_t inode_table;  // The block containing the inode table

    uint16_t free_inodes; // The number of free inodes in this block group
    uint16_t free_blocks; // The number of free blocks in this block group
    uint16_t dir_count;   // The number of directories currently in the block group

    uint8_t reserved[30]; // Reserved for future use

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
    uint64_t access_time;       // The time the inode was last accessed

    uint64_t byte_size;   // The size of the file in bytes
    uint64_t block_count; // The number of blocks the file uses

    uint64_t extents[1 + 2 * 4]; // The extent tree for the file (or just raw data for inline files)

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

// Check that the size of the tree node (and friends) are correct
static_assert(sizeof(cfs_tree_node_t) == 4 + CFS_TREE_ORDER * 8 + (CFS_TREE_ORDER * CFS_KEY_SIZE),
              "The size of the tree node is incorrect");
static_assert(sizeof(dir_entry_t) == CFS_TREE_ORDER * 8,
              "The size of the directory entry is incorrect");
static_assert(sizeof(cfs_tree_key_t) == CFS_KEY_SIZE,
              "The size of the key is incorrect");

#endif // CANDYFS_H
