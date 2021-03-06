#include <efi.h>
#include <types.h>
#pragma pack (1)

struct gdt_entry {
  uint16_t limit15_0;            uint16_t base15_0;
  uint8_t  base23_16;            uint8_t  type;
  uint8_t  limit19_16_and_flags; uint8_t  base31_24;
};

__attribute__((aligned(4096)))
struct {
  struct gdt_entry null;
  struct gdt_entry kernel_code;
  struct gdt_entry kernel_data;
  struct gdt_entry null2;
  struct gdt_entry user_data;
  struct gdt_entry user_code;
  struct gdt_entry ovmf_data;
  struct gdt_entry ovmf_code;
  struct gdt_entry tss_low;
  struct gdt_entry tss_high;
} gdt_table = {
    {0, 0, 0, 0x00, 0x00, 0},  /* 0x00 null  */
    {0, 0, 0, 0x9a, 0x20, 0},  /* 0x08 kernel code (kernel base selector) */
    {0, 0, 0, 0x92, 0x00, 0},  /* 0x10 kernel data */
    {0, 0, 0, 0x00, 0x00, 0},  /* 0x18 null (user base selector) */
    {0, 0, 0, 0xF2, 0x00, 0},  /* 0x20 user data */ //Type: 0b11110010; Acc: 0b00000000
    {0, 0, 0, 0xfa, 0x20, 0},  /* 0x28 user code */ //Type: 0b11111010; Acc: 0b00100000
    {0, 0, 0, 0x92, 0xa0, 0},  /* 0x30 ovmf data (not used) */
    {0, 0, 0, 0x9a, 0xa0, 0},  /* 0x38 ovmf code (not used)*/
    {0, 0, 0, 0x89, 0xa0, 0},  /* 0x40 tss low */
    {0, 0, 0, 0x00, 0x00, 0},  /* 0x48 tss high */
};

__attribute__((aligned(4096)))
struct tss {
    uint32_t reserved0; uint64_t rsp0;      uint64_t rsp1;
    uint64_t rsp2;      uint64_t reserved1; uint64_t ist1;
    uint64_t ist2;      uint64_t ist3;      uint64_t ist4;
    uint64_t ist5;      uint64_t ist6;      uint64_t ist7;
    uint64_t reserved2; uint16_t reserved3; uint16_t iopb_offset;
} tss;

#pragma pack ()

#pragma pack (1)

struct table_ptr {
    uint16_t limit;
    uint64_t base;
};

#pragma pack ()