echo "Compiling efi part"
GNU_EFI_DIR="/mnt/d/gnu-efi/gnu-efi"

gcc boot/efi.c                         \
    -c                                 \
    -fno-stack-protector               \
    -fpic                              \
    -fshort-wchar                      \
    -mno-red-zone                      \
    -I $GNU_EFI_DIR/inc                \
    -I $GNU_EFI_DIR/inc/x86_64         \
    -I efikernel/header                \
    -DEFI_FUNCTION_WRAPPER             \
    -ggdb                              \
    -o build/efi.o

nasm boot/loadgdt.asm -o build/loadgdt.o -f elf64

ld build/efi.o                     \
    build/loadgdt.o                \
    $GNU_EFI_DIR/x86_64/gnuefi/crt0-efi-x86_64.o \
    -nostdlib                      \
    -znocombreloc                  \
    -T $GNU_EFI_DIR/gnuefi/elf_x86_64_efi.lds \
    -shared                        \
    -Bsymbolic                     \
    -L $GNU_EFI_DIR/x86_64/lib     \
    -L $GNU_EFI_DIR/x86_64/gnuefi  \
    -l:libgnuefi.a                 \
    -l:libefi.a                    \
    -o build/efi.so

objcopy -j .text                \
        -j .sdata               \
        -j .data                \
        -j .dynamic             \
        -j .dynsym              \
        -j .rel                 \
        -j .rela                \
        -j .reloc               \
        -j .debug_info          \
        -j .debug_abbrev        \
        -j .debug_loc           \
        -j .debug_aranges       \
        -j .debug_line          \
        -j .debug_macinfo       \
        -j .debug_str           \
        --target=efi-app-x86_64 \
        build/efi.so            \
        build/debug.efi

objcopy -j .text                \
        -j .sdata               \
        -j .data                \
        -j .dynamic             \
        -j .dynsym              \
        -j .rel                 \
        -j .rela                \
        -j .reloc               \
        --target=efi-app-x86_64 \
        build/efi.so            \
        build/BOOTX64.EFI
