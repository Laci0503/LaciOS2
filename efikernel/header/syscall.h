#ifndef _SYSCALL_H
#define _SYSCALL_H
#include <types.h>

volatile typedef struct syscall_args_struct
{
    uint64 rip;
    uint64 main_class;
    uint64 sub_class;
    uint64 arg_0;
    uint64 arg_1;
    uint64 arg_2;
    uint64 arg_3;
    uint64 rflags;
    uint64 rsp;
    uint64 retval0;
    uint64 retval1;
} syscall_args_struct;

extern syscall_args_struct syscall_args;

extern void syscall_entry(void);
void syscall_handler();

#endif