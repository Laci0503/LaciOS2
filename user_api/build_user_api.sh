FILES="
    
"
#not ideal flags I think but okay for now
GCCFLAGS="
    -ffreestanding \
    -fno-pie \
    -mcmodel=large \
    -mno-red-zone \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -c \
    -Iuser_api/header \
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
    gcc $GCCFLAGS user_api/src/$file.c -o build/user_api/$file.o
done

nasm user_api/src/syscalls.asm -o build/user_api/syscalls.o -f elf64

#objfiles="build/"
#
#for file in $FILES
#do
#    objfiles=$objfiles\ build/$file.o
#done
#
#ld -m elf_x86_64 -Tefikernel/linker.ld $objfiles -o efifilesystem/kernel.bin -nostdlib --oformat binary