IMAGE=test.img
SIZE=1G
alias GPTIMG="gptimg/bin/gptimg"

# Create the image
echo "Creating disk image..."
GPTIMG create "$IMAGE" --size "$SIZE"

# Add partitions
echo "Adding partitions to disk image..."
esp_partition=$(GPTIMG add-partition "$IMAGE" --size 33M --type efi --name "EFI SYSTEM")
data_partition=$(GPTIMG add-partition "$IMAGE" --size 100M --type basic-data --name "BASIC DATA")
$(GPTIMG add-partition "$IMAGE" --size 100M --type basic-data --name "BASIC DATA")

# Setup the loopback device and mount file
echo "Setting up loopback device..."
loop_device=$(losetup -f)
mount_point=$(mktemp -d)
sudo losetup "$loop_device" "$IMAGE" -P

# Format the partitions
echo "Formatting partitions..."
sudo mkfs.fat -F 32 "$loop_device"p"$esp_partition" # ESP -> FAT32
sudo mkfs.ext4 "$loop_device"p"$data_partition"     # OS  -> EXT4

# Add EFI/BOOT/BOOTX64.EFI to the ESP
BOOT_FILE=boot/bootloader/BOOTX64.EFI
echo "compiling BOOTX64.EFI"
cd boot/bootloader && make -s full && cd ../..

sudo mount "$loop_device"p"$esp_partition" "$mount_point"
sudo mkdir -p $mount_point/EFI/BOOT
sudo cp $BOOT_FILE $mount_point/EFI/BOOT/BOOTX64.EFI
sudo umount "$mount_point"

# Add files to the OS partition
sudo mount "$loop_device"p"$data_partition" "$mount_point"
sudo mkdir -p $mount_point/boot
sudo echo "Hello world!" > $mount_point/boot/kernel.elf
sudo umount "$mount_point"

# Detatch the loop device
sudo losetup -d "$loop_device"

# Add file permissions
chmod a+rw $IMAGE
