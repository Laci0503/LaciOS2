#include <syscall.h>
#include <task_scheduler.h>
#include <basic_functions.h>

void syscall_handler(){
    print_to_serial("SYSCALL\n\r");
    
}