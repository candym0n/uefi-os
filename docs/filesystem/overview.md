# Filesystem Overview

CandyFS is the primary filesystem of this OS, designed for simplicity, robustness, and good performance on UEFI-based machines.

## Goals

- Simple on-disk format based on Ext2
- Extent-based allocation for fewer seeks
- Clear seperation between VFS and CandyFS implementation

## Integration with the OS
- Mounted as the root filesystem at boot
- Exposed through the OS VFS as standard file operations
- Bootloader passes a block device hande; the kernel mounts CandyFS from that device
