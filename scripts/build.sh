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
data_partition=$(GPTIMG add-partition "$IMAGE" --size 1M --type basic-data --name "BASIC DATA")

# Format the ESP
echo "Formatting partitions..."
GPTIMG format "$IMAGE" --partition $esp_partition

# Mount the ESP
echo "Adding files to ESP (partition $esp_partition)"
mount_point=$(mktemp -d)
loop_device=$(losetup -f)
sudo losetup "$loop_device" "$IMAGE" -P
sudo mount "$loop_device"p1 "$mount_point"

# Add EFI/BOOT/BOOTX64.EFI
echo "compiling BOOTX64.EFI"
BOOT_FILE=boot/bin/BOOTX64.EFI
cd boot && make -s full && cd ..
sudo mkdir $mount_point/EFI
sudo mkdir $mount_point/EFI/BOOT
sudo cp $BOOT_FILE $mount_point/EFI/BOOT/BOOTX64.EFI

# Unmount the loop device
sudo umount "$mount_point"
sudo losetup -d "$loop_device"
