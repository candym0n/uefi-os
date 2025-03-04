# UEFI based OS
This project is a simple, UEFI-based OS.

## Filesystem
The filesystem this OS is planned to support is FAT32 and Candy FS (a custom filesystem). Candy FS is going to be sort of based off of Ext2, but with extra features like extents to improve speed and efficiency.

## Building
First, you must install the neccessary things.
```
sudo apt install gcc ld gnu-efi make qemu-system-x86_64
```
Now all you have to do is run
```
make    # Note that you'll need superuser permission (for loopdevices and such)
```
to build and
```
make run
```
to emulate in qemu. If this doesn't work, please submit an issue. I also plan to support most versions of linux :)

NOTE: Technically you don't need to run it using qemu. The resulting image is placed in build/test.img so you could use another virtual machine if you wanted, or even use something like Rufus to load it onto a USB and boot it up on a real computer.
