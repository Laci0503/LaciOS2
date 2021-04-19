#ifndef _SYSCALLS_H
#define _SYSCALLS_H
#include <types.h>

extern uint64 syscall_wrapper(uint64 main_class, uint64 sub_class, uint64 arg_0, uint64 arg_1, uint64 arg_2, uint64 arg_3);

#endif