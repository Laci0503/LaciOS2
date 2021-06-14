#ifndef _SYSCALL_H
#define _SYSCALL_H
#include <types.h>

#define SYSCALL_DEBUG 0 // Syscall handler debug output

#define SYSCALL_IO 0
#define SYSCALL_IO_SERIAL 0
#define SYSCALL_IO_READ_BYTE 0
#define SYSCALL_IO_WRITE_BYTE 1
#define SYSCALL_IO_READ_STRING 2
#define SYSCALL_IO_WRITE_STRING 3

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
void syscall_init();
void syscall_io_serial();
/*void (*syscall_io_handlers[])(void);
void** syscall_handlers[];*/

#endif