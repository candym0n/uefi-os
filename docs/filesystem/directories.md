# Directories in Candy FS
Directories in CandyFS are regular inodes whose data consists of fixed-sized directory entries (cfs_dir_entry_t). Each entry stores a filename and the inode number of the file-object it refers to.

## Directory Inode Basics
A directory is represented by a `cfs_inode_t` with `mode.type == DIRECTORY`.

### Key fields:
- `mode.type`: Must be `DIRECTORY`
- `byte_size`: Total size in bytes of all directory entries (`number of entries * sizeof(cfs_dir_entry_t)`)
- `block_count`: Number of blocks used by the directory (`ceil(byte_size / BLOCK_SIZE)`)
- `extents`: Describes where the directory's data blocks live on disk, or holds inline data.

The directory inode lives in the same inode table as all other file types and is located using the filesystem's group descriptor and inode indexing scheme, just like a regular file.

## Directory Entry Format
Directory contents are a flat array of `dir_entry_t` records stored in the directory's data area.

### Key fields:
- `name`
    - UTF-8 string, null-terminated
    - Up to `CFS_NAME_LEN - 1` characters
    - Unused bytes after terminating `\0` are padding
- `inode`
    - 0 means the entry is free / not in use
    - Non-zero is the global inode index of the referenced file or directory

To search or iterate entries, simply walk through each entry in steps of `sizeof(dir_entry_t)` until you've found it. Trivial!

## Creating and deleting directory entries
### Creating an entry
To add an entry `<name, inode>` to a directory:
1. Append a new `dir_entry_t` at `byte_offset = dir->byte_size`.
2. Allocate a new block or extend an extent as needed
3. Fill in metadata for the entry (`name`, `inode`)

This relies on the fact that there are no gaps in the unused entries.

### Deleting an entry
To remove an entry:
1. Iterate through the directory to find the `dir_entry_t` with the given name
2. Replace the entry's metadata with that of the last entry (at `dir->byte_size`)
3. For the last entry, set `ent->inode = 0` and, optionally, `ent->name[0] = 0` for cleanliness
4. Shrink the directory if less blocks are needed
