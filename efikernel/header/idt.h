#ifndef _IDT_H
#define _IDT_H
#include <types.h>
#include <config.h>

typedef struct{
    uint16 offset_1;
    uint16 selector;
    uint8 interrupt_stack_table_selector;
    uint8 type_attr;
    uint16 offset_2;
    uint32 offset_3;
    uint32 resv;
} __attribute__((packed)) idt_entry;

idt_entry IDT[256];

typedef struct{
    uint16 limit;
    uint64 base;
} __attribute__((packed)) idt_descriptor;

idt_descriptor IDT_DESC;


//interrupt_stack_table_selector:
//  7                           0
//+---+---+---+---+---+---+---+---+
//|        Zero       |    IST    |
//+---+---+---+---+---+---+---+---+
//
//type_attr:
//  7                           0
//+---+---+---+---+---+---+---+---+
//| P |  DPL  | Z |    GateType   |
//+---+---+---+---+---+---+---+---+

void init_idt();

typedef struct{
    uint64 rax;
    uint64 rbx;
    uint64 rcx;
    uint64 rdx;
    uint64 rsi;
    uint64 rdi;
    uint64 rbp;
    uint64 r8;
    uint64 r9;
    uint64 r10;
    uint64 r11;
    uint64 r12;
    uint64 r13;
    uint64 r14;
    uint64 r15;
    uint8 RESV_FOR_FLOAT[80];
    uint8 RESV_FOR_XMM[256];
    uint64 error_code;
    uint64 rip;
    uint64 cs;
    uint64 rflags;
    uint64 rsp;
    uint64 ss;
} CPU_state;

extern CPU_state current_state;

#if(IDT_DEBUG_OUTPUT)
    uint64 test_idt;
#endif

#endif