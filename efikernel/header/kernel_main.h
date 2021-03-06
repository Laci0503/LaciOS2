#ifndef __KERNEL_MAIN_H
#define __KERNEL_MAIN_H
#include <types.h>
#include <gdt.h>
#include <config.h>

typedef struct {
    uint32          Type;           // Field size is 32 bits followed by 32 bit pad
    uint32          Pad;
    uint64          PhysicalStart;  // Field size is 64 bits
    uint64          VirtualStart;   // Field size is 64 bits
    uint64          NumberOfPages;  // Field size is 64 bits
    uint64          Attribute;      // Field size is 64 bits
} EFI_MEMORY_DESCRIPTOR;

typedef struct{
    uint64 real_address;
    uint64 framebuffer;
    uint64 physical_framebuffer;
    uint64 screen_width;
    uint64 screen_height;
    uint64 kernel_pml4_addresss;
    EFI_MEMORY_DESCRIPTOR* memmap;
    uint64 memmap_desc_count;
    uint64 memmap_desc_size;
    uint64 largest_area_idx;
    uint64 used_pages;
    GDT* gdt;
    TSS* tss;
    uint64 acpi_rsdp;
    uint64 io_space_used_pages;
} kernel_info;

void kernel_main(kernel_info* kernel_info);
void kernel_panic(char* error);

#endif