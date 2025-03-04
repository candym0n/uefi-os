#!bin/bash

#TARGET=build/test.img
#SIZE=1G
GPTIMG="$GPTIMG_DIR/build/gptimg"

# Create the image
echo "Creating disk image..."
$GPTIMG create "$TARGET" --size "$SIZE"

# Add partitions
echo "Adding partitions to disk image..."
esp_partition=$($GPTIMG add-partition "$TARGET" --size 64M --type efi --name "EFI System Partition")
os_partition=$($GPTIMG add-partition "$TARGET" --size 512M --type basic-data --name "Operating System")
data_partition=$($GPTIMG add-partition "$TARGET" --size 256M --type basic-data --name "Basic Data")

# Setup the loopback device and mount file
echo "Setting up loopback device..."
loop_device=$(losetup -f)
mount_point=$(mktemp -d)
sudo losetup "$loop_device" "$TARGET" -P

# Format the partitions
echo "Formatting partitions..."
sudo mkfs.fat -F 32 "$loop_device"p"$esp_partition" # ESP -> FAT32

# Add EFI/BOOT/BOOTX64.EFI to the ESP (using a mount sandwich)
echo "Adding bootloader to EFI System Partition..."
BOOT_FILE=$BOOT_DIR/build/BOOTX64.EFI
sudo mount "$loop_device"p"$esp_partition" "$mount_point"   # Top bun (mount)
sudo mkdir -p $mount_point/EFI/BOOT                         # Meat (create dir)
sudo cp $BOOT_FILE $mount_point/EFI/BOOT/BOOTX64.EFI        # Meat (copy file)
sudo umount "$mount_point"                                  # Bottom bun (unmount)

# Detatch the loop device and get rid of the mount point
sudo losetup -d "$loop_device"
sudo rm -r "$mount_point"

# Change file permissions
sudo chmod a+rw $TARGET
