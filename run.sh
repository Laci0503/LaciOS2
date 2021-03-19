sh build.sh
#qemu-system-x86_64 -bios bios64.bin -drive file=build/fat.img,id=fat,if=none,format=raw -usb -device usb-storage,drive=fat
qemu-system-x86_64 -bios OVMF.fd -hda image/LaciOS.img -m 1G -d int,cpu_reset,mmu,page