FILES="
    memory \
    video \
    basic_functions \
    idt \
    task_scheduler \
    syscall \
    kernel_main \
"
GCCFLAGS="
    -ffreestanding \
    -fno-pie \
    -mcmodel=large \
    -mno-red-zone \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -c \
    -Iefikernel/header \
    -ffunction-sections \
"

#-mno-sse \
#-mno-sse2 \

for file in $FILES
do
    rm build/$file.o
done

for file in $FILES
do
    gcc $GCCFLAGS efikernel/src/$file.c -o build/$file.o
done

nasm efikernel/src/assembly_functions.asm -o build/assembly_functions.o -f elf64

objfiles="build/assembly_functions.o"

for file in $FILES
do
    objfiles=$objfiles\ build/$file.o
done

ld -m elf_x86_64 -Tefikernel/linker.ld $objfiles -o efifilesystem/kernel.bin -nostdlib --oformat binary


