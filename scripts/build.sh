IMAGE=test.img
SIZE=1G
alias GPTIMG="gptimg/bin/gptimg"

echo $GPTIMG

# Create the image
echo "Creating disk image..."
GPTIMG create "$IMAGE" --size "$SIZE"

# Add partitions
echo "Adding partitions to disk image..."
esp_partition=$(GPTIMG add-partition "$IMAGE" --size 33M --type efi --name "EFI SYSTEM")
data_partition=$(GPTIMG add-partition "$IMAGE" --size 100M --type basic-data --name "BASIC DATA")

# Setup the loopback device and mount it
echo "Setting up loopback device..."
mount_point=$(mktemp -d)
loop_device=$(losetup -f)
sudo losetup "$loop_device" "$IMAGE" -P

# Format the partitions
echo "Formatting partitions..."
sudo mkfs.fat -F 32 "$loop_device"p"$esp_partition"
sudo mkfs.ext4 "$loop_device"p"$data_partition"

# Add EFI/BOOT/BOOTX64.EFI
sudo mount "$loop_device"p1 "$mount_point"
echo "compiling BOOTX64.EFI"
BOOT_FILE=boot/bootloader/BOOTX64.EFI
cd boot/bootloader && make -s full && cd ../..
sudo mkdir -p $mount_point/EFI/BOOT
sudo cp $BOOT_FILE $mount_point/EFI/BOOT/BOOTX64.EFI
sudo umount "$mount_point"

# Detatch the loop device
sudo losetup -d "$loop_device"
