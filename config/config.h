#ifndef _CONFIG_H
#define _CONFIG_H // /config/config.h

//Memory
#define KERNEL_VMA_PDPT                 4       // Start pdpt of the kernel VMA space. Kernel VMA space is 1 pdpt (512 GiB).
#define KERNEL_VMA_ADDRESS (KERNEL_VMA_PDPT * 0x8000000000ULL) // Start address of the kernel VMA space.
#define KERNEL_BINARY_PD                0       // Start PD of the kernel binary in the kernel VMA space.
#define KERNEL_INFO_START_PD            128     // Start PD of the kernel_info structure.
#define KERNEL_STACK_START_PD           256     // Start PD of the kernel stack.
#define KERNEL_HEAP_START_PD            384     // Start PD of the kernel heap.
#define KERNEL_STACK_SIZE               1024    // Kernel stack size in pages. Max: 33,554,432 page (128 GiB) 1 page = 4096 bytes
#define KERNEL_SECONDARY_STACK_SIZE     1024    // Secondary kernel stack (Used for interrupt and syscall handlers) size in pages. Max: 33,554,432 page (128 GiB) 1 page = 4096 bytes
#define KERNEL_HEAP_SIZE                25600   // Kernel heap size in pages. Max: 33,554,432 page (128 GiB) 1 page = 4096 bytes
#define PAGE_ADDR_MASK 0x000ffffffffff000       // Address mask for 4096 byte (1 page) alignment

#define MEMORY_IO_PDPT                  5       // The pdpt reserved for the memory mapped I/O devices

//Debug
#define ACPI_DEBUG 1                            // Debug output at ACPI parsing
#define IDT_DEBUG_OUTPUT 0                      // Debug output on interrupts
#define EFI_DEBUG 1                             // Debug output at booting

//IDT
#define HardwareInterruptOffset 0x20            // Interrupt number offset of hardware generated interrupts
#define PICMasterCommand 0x20                   // Master PIC command port
#define PICMasterData 0x21                      // Master PIC data port
#define PICSlaveCommand 0xA0                    // Slave PIC command port
#define PICSlaveData 0xA1                       // Slave PIC data port

//General ports
#define KEYBOARD_PORT 0x60                      // I/O Port of the PS2 keyboard
#define SERIAL_PORT 0x3F8                       // I/O Port of the first serial port


#endif