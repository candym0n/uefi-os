# CandyFS On-Disk Layout

This document describes the exact on-disk structures used by CandyFS

## Global layout

- The disk is divided into fixed-size blocks (e.g. 4 KiB)
- Blocks are grouped into block groups, each managing its own bitmaps and inode table
- The first block of the volume contains the superblock

| Group # | Type       | Span     | Contains                |
|---------|------------|----------|-------------------------|
| 0       | Superblock | 1 block  | Global metadata + MBR   |
| 0       | GDT        |`gdt_span`| Group descriptors       |
| 0       |Data blocks | Leftover |Can be used to store data|
| 1       |Block Bitmap| 1 block  | Marks usage of blocks   |
| 1       |Inode Bitmap| 1 block  | Marks usage of inodes   |
| 1       |Inode Table | See NOTE | Describes group's inodes|
| 1       |Data blocks | Leftover |Can be used to store data|
| ...     |...         | ...      |...                      |

- NOTE: Yes, the first block group is special, because it has the superblock and GDT taking up its first few blocks.
- NOTE: `INODES_PER_GROUP * sizeof(cfs_inode_t) / BLOCK_SIZE`

## Superblock
The first block (block #0) is the superblock. It contains metadata about the entire volume.

### Key fields (`cfs_superblock_t`):
- `name`: UTF-16 volume name (up to 32 units, or 64 bytes)
- `uuid`: 16 byte UUID for the FS
- `magic`: Must be `0xCAFE` for CandyFS
- `inodes_count`, `blocks_count`, `group_count`: Number of inodes, blocks, and block groups in the entire volume
- `block_size`: Number of bytes in a block (e.g. 4096 for 4 KiB)
- `block_group_size`: Number of blocks in a block group
- `gdt_start`: First physical block of the GDT
- `gdt_span`: Number of blocks the GDT spans
- `root_inode_index`, `bad_blks_inode_index`: The index of the root and bad blocks inodes
- `checksum`: CRC32 checksum for the entire superblock (minus this field)

If `magic` does not match, the mount attempt fails

## Block Groups and the GDT

The second block (block #1) is the Group Descriptor Table or GDT. It provides metadata information for every single block group afterwards. It is composed of several consecutive group descriptors.

### Key Fields (`cfs_group_desc_t`)
- `block_bitmap`: The physical block containing the block bitmap
- `inode_bitmap`: The physical block containing the inode bitmap
- `inode_table`: The physical block where the inode table starts
- `free_inodes`, `free_blocks`, `dir_count`

NOTE: A physical block index of 0 means uninitialized; that's the superblock, dummy!

### Contents of a block group

Each block group has the following "special" blocks, the location of which determined by its entry in the GDT:

- Block bitmap
- Inode bitmap
- Inode table (spans several consecutive blocks)

### Block and Inode Bitmaps
In every block group there are 2 bitmaps. One is the block bitmap, and the other is the inode bitmap. Each of these bitmaps take up exactly one block, which means there is a maximum of `8 * BLOCK_SIZE` blocks in a block group, if `BLOCK_SIZE` is the size of a block in bytes. Both are little endian bitmaps, where 0 means free, and 1 means used.

### Inode Table
The inode table is composed of several consecutive `cfs_inode_t`,  `INODES_PER_GROUP` of them to be exact. That means that this inode table will span `INODES_PER_GROUP * sizeof(cfs_inode_t) / BLOCK_SIZE` blocks in this block group (rounded up). Inodes are discussed in more detail in the inodes portion of this document.

## Inodes
Each inode stores information about a file-system object such as a file or directory. They are described in the inode table of a block group, and has data in the data blocks of any block group.

### Key Fields (`cfs_inode_t`)
- `mode`: Types and permissions of the inode
- `user`: The user that owns the inode
- `group`: The group that the inode is a part of
- `hard_links_count`: The number of hard links to the inode
- `byte_size`: The size of the file in bytes
- `block_count`: The number of blocks the file uses
- `extents`: Extent tree for the file (or raw data for inline files)
- `checksum`: Checksum for the inode
- `creation_time`, `modification_time`, `access_time`

### Extents
Extents are used to store data in contiguous blocks. For example, instead of saying "This file has data from blocks 1, 2, 3, 4, 10, 11, 12" you can just say "This file has data in the range [1, 4] U [10, 12]". 

### Extent tree layout
This pattern is used both in the inode (`extents` field) and in extent tree blocks.

- `cfs_extent_header_t` - Provides information on depth and entry count
- A list of a bunch of leaf or internal extent nodes
    - `cfs_extent_id_t` - Internal, points to a whole block that follows this extent-defining pattern
        - `logical_block` - The first logical block this extent defines (every child extent starts from here)
        - `physical_block` - The physical block containing the extent tree
    - `cfs_extent_t` - Leaf, points to a range of data that will be defined for this file
        `logical_block` - The first logical block this extent covers (minus any offset from parent nodes)
        `physical_block` - The first physical block of the extent
        `count` - The number of blocks covered by this extent
