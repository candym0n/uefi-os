qemu-system-x86_64 \
-drive format=raw,file=test.img \
-bios bios64.bin \
-m 256M \
-vga std \
-name TESTOS \
-machine q35 \
-usb \
-device usb-mouse \
-rtc base=localtime \
-net none