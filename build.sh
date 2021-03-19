#!/bin/bash

mkdir -p build
mkdir -p efifilesystem
mkdir -p image
mkdir -p build/iso
mkdir -p efifilesystem/EFI
mkdir -p efifilesystem/EFI/BOOT
mkdir -p efifilesystem/lacios

sh boot/buildefi.sh
sh efikernel/buildefikernel.sh

rm image/LaciOS.img image/fat.img

rm efifilesystem/EFI/BOOT/BOOTX64.EFI || echo ""
cp build/BOOTX64.EFI efifilesystem/EFI/BOOT/BOOTX64.EFI

dirs=$(find efifilesystem/ ! -path efifilesystem/ -type d | awk -F/ '{ for(i=2; i<=NF; i++) {printf("/%s", $i)}print ""}')
files=$(find efifilesystem/  -type f | awk -F/ '{ for(i=2; i<=NF; i++) {printf("/%s", $i)}print ""}')

echo $files
echo $dirs

dd if=/dev/zero of=image/LaciOS.img bs=1M count=128
parted image/LaciOS.img -s -a minimal mklabel gpt
parted image/LaciOS.img -s -a minimal mkpart EFI FAT32 2048s 245760s
parted image/LaciOS.img -s -a minimal toggle 1 boot
dd if=/dev/zero of=image/fat.img bs=512 count=245760
mformat -i image/fat.img -h 120 -t 32 -n 64 -c 1 -F

for dir in $dirs
do
    echo "mmd"
    mmd -i image/fat.img ::$dir
done

for file in $files
do
    echo "mcopy"
    mcopy -i image/fat.img efifilesystem$file ::$(dirname "${file}")
done

dd if=image/fat.img of=image/LaciOS.img bs=512 count=245760 seek=2048 conv=notrunc

if [ -z "$1" ]; then
echo ""
else
    cp image/fat.img build/iso/BOOTX64.img
    xorriso -as mkisofs -R -f -e BOOTX64.img -no-emul-boot -o image/LaciOS.iso build/iso 
fi