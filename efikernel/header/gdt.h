#ifndef _GDT_H
#define _GDT_H
#include <types.h>

#pragma pack (1)
typedef struct{
    uint16 limit15_0;            uint16 base15_0;
    uint8  base23_16;            uint8  type;
    uint8  limit19_16_and_flags; uint8  base31_24;
} gdt_entry;
#pragma pack ()

#pragma pack (1)
__attribute__((aligned(4096)))
typedef struct {
    uint32 reserved0; uint64 rsp0;      uint64 rsp1;
    uint64 rsp2;      uint64 reserved1; uint64 ist1;
    uint64 ist2;      uint64 ist3;      uint64 ist4;
    uint64 ist5;      uint64 ist6;      uint64 ist7;
    uint64 reserved2; uint16 reserved3; uint16 iopb_offset;
} TSS;
#pragma pack ()
#pragma pack (1)
typedef struct {
    gdt_entry null;
    gdt_entry kernel_code;
    gdt_entry kernel_data;
    gdt_entry null2;
    gdt_entry user_data;
    gdt_entry user_code;
    gdt_entry ovmf_data;
    gdt_entry ovmf_code;
    gdt_entry tss_low;
    gdt_entry tss_high;
} GDT;
#pragma pack ()

#pragma pack(1)
typedef struct {
    uint16 limit;
    uint64 base;
} gdt_ptr;
#pragma pack()

GDT* gdt;
TSS* tss;

#endif