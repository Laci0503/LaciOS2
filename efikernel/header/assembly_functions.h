#ifndef _ASSEMBLY_FUNCTIONS_H
#define _ASSEMBLY_FUNCTIONS_H
#include <types.h>
#include <memory.h>

extern float80 sin(float80 number);
extern void interrupts_int0(void);
extern void interrupts_int1(void);
extern void while1(void);
extern void enable_sce(void);
extern void enter_userspace(uint64 rip, uint64 rsp, uint64 rflags);
extern void load_gdt(void* gdt_pointer);

extern void cpl3_kernel_part;
extern struct{
    uint64 cpl3_rax;
    uint64 cpl3_rcx;
    uint64 cpl3_r11;
    uint64 cpl3_rip;
    uint64 cpl3_rflags;
} cpl3_reg_save;
extern void cpl3_returner(void);

extern void load_pml4(page_map_level_4* pml4);

#endif