ENTRY(kernel_main)
SECTIONS
{
    . = 0x20000000000;
    .text : AT(ADDR(.text) - 0x20000000000)
    {
        _code = .;
        *(.text.kernel_main)
        *(.text*)
        . = ALIGN(4096);
    }

   .data : AT(ADDR(.data) - 0x20000000000)
   {
        _data = .;
        *(.data)
        *(.rodata*)
        . = ALIGN(4096);
   }

   .eh_frame : AT(ADDR(.eh_frame) - 0x20000000000)
   {
       _ehframe = .;
       *(.eh_frame)
        . = ALIGN(4096);
   }

   .bss : AT(ADDR(.bss) - 0x20000000000)
   {
       _bss = .;
       *(.bss)
        *(COMMON)
       . = ALIGN(4096);
   }

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
        *(.note.gnu.property)
   }
}