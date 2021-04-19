#include <syscall.h>
#include <task_scheduler.h>
#include <basic_functions.h>

void syscall_handler(){ // Optimise: instead of switch, create an array of function pointers
    #if(SYSCALL_DEBUG)
        print_to_serial("SYSCALL\n\r");
        print_to_serial("main_class: ");
        print_int_to_serial(syscall_args.main_class);
        print_to_serial("\n\rsub_class: ");
        print_int_to_serial(syscall_args.sub_class);
        print_to_serial("\n\rarg_0: ");
        print_int_to_serial(syscall_args.arg_0);
        print_to_serial("\n\rarg_1: ");
        print_int_to_serial(syscall_args.arg_1);
        print_to_serial("\n\rarg_2: ");
        print_int_to_serial(syscall_args.arg_2);
        print_to_serial("\n\rarg_3: ");
        print_int_to_serial(syscall_args.arg_3);
        print_to_serial("\n\rrflags: ");
        print_bin_to_serial(syscall_args.rflags);
        print_to_serial("\n\r");
    #endif
    switch (syscall_args.main_class)
    {
    case SYSCALL_IO: // main_class: I/O
        switch (syscall_args.sub_class)
        {
        case SYSCALL_IO_SERIAL: // sub_class: serial
            syscall_io_serial();
            break;
        
        default:
            break;
        }
        break;
    
    default:
        break;
    }
}

void syscall_init(){

}

void syscall_io_serial(){
    if(syscall_args.arg_0==SYSCALL_IO_WRITE_BYTE){
        outb(SERIAL_PORT,syscall_args.arg_1);
        return;
    }
    if(syscall_args.arg_0==SYSCALL_IO_WRITE_STRING){
        print_to_serial(syscall_args.arg_1);
    }
}