#include <types.h>
#include <kernel_main.h>
#include <basic_functions.h>
#include <video.h>
#include <assembly_functions.h>
#include <memory.h>
#include <gdt.h>
#include <idt.h>
#include <task_scheduler.h>
#include <syscall.h>
#include <acpi.h>
#include <device_manager.h>
#include <collections.h>
#include "../../user_api/header/syscalls.h"

void usercode();
void usercode2();
uint8* kernel_stack_real_addr;
void* kernel_stack_vma;

void kernel_main(kernel_info* kernel_info){

    //print_to_serial("THIS IS KERNEL YEEEY!");

    framebuffer=(rgb*)kernel_info->framebuffer;
    width=kernel_info->screen_width;
    height=kernel_info->screen_height;
    rgb red={0,0,255,0};

    init_memory_manager(kernel_info);
    
    #if(SCREEN_DEBUG)
        print_to_serial("Screen width: ");
        print_int_to_serial(width);
        print_to_serial("\n\r");
        print_to_serial("Screen height: ");
        print_int_to_serial(height);
        print_to_serial("\n\r");
    #endif

    /*print_to_serial("RSDP2 PTR: ");
    print_hex_to_serial(kernel_info->acpi_rsdp);
    print_to_serial("\n\r");
    uint64 rsdp2_address = kernel_info->acpi_rsdp;*/

    init_idt();
    
    linked_list* list=malloc(sizeof(linked_list));
    memset(list, sizeof(linked_list), 0);

    uint64 one = 1;
    uint64 two = 2;
    uint64 three = 3;
    uint64 four = 4;
    uint64 five = 5;

    linked_list_item* first = linked_list_add(list, &one);
    linked_list_item* second = linked_list_add(list, &two);
    linked_list_item* third = linked_list_add(list, &three);

    print_to_serial("First item: ");
    print_int_to_serial(*((uint64*)(list->first->item)));
    print_to_serial("\n\ritem at index 0: ");
    print_int_to_serial(*((uint64*)(linked_list_get(list, 0)->item)));
    print_to_serial("\n\ritem at index 1: ");
    print_int_to_serial(*((uint64*)(linked_list_get(list, 1)->item)));
    print_to_serial("\n\ritem at index 2: ");
    print_int_to_serial(*((uint64*)(linked_list_get(list, 2)->item)));
    print_to_serial("\n\rlast item: ");
    print_int_to_serial(*((uint64*)(list->last->item)));
    print_to_serial("\n\rcount: ");
    print_int_to_serial(list->count);
    print_to_serial("\n\r");

    linked_list_remove_item(list, second);
    print_to_serial("First item: ");
    print_int_to_serial(*((uint64*)(list->first->item)));
    print_to_serial("\n\ritem at index 0: ");
    print_int_to_serial(*((uint64*)(linked_list_get(list, 0)->item)));
    print_to_serial("\n\ritem at index 1: ");
    print_hex_to_serial((uint64)linked_list_get(list, 1));
    print_to_serial("\n\rlast item: ");
    print_int_to_serial(*((uint64*)(list->last->item)));
    print_to_serial("\n\rcount: ");
    print_int_to_serial(list->count);
    print_to_serial("\n\r");

    /*enable_sce();
    print_to_serial("&usercode: ");
    print_hex_to_serial((uint64)usercode);
    print_to_serial("\n\rTss: ");
    print_hex_to_serial((uint64)tss);
    print_to_serial("\n\rGdt: ");
    print_hex_to_serial((uint64)gdt);
    print_to_serial("\n\r");

    init_task_scheduler();
    int64 idx=add_task(usercode,4096,0);
    tasks[idx].state=RUNNING;
    //asm("cli")
    //print_to_serial("Before init devices\n\r");
    init_devices((RSDPDescriptor20*)rsdp2_address);
    //print_to_serial("After init devices\n\r");

    start_task_scheduler();

    //init_video();
    //draw_rectangle(0,0,100,100,torgb(255,0,0));
    //framebuffer[800*600-1]=torgb(255,0,0);
    //print_to_serial("End\n\r");
    //print_to_serial("After init devices\n\r");*/
    while(1);
}

void usercode(){
    //asm("SYSCALL" : : );
    //while(1);
    char letter='A';
    char string[]="Test string\n\r";
    while(1){
        if(letter>'Z')letter='A';
        //asm volatile("outb %0, %1" : : "a" (letter), "Nd" (SERIAL_PORT));
        //asm volatile("cli");
        syscall_wrapper(SYSCALL_IO,SYSCALL_IO_SERIAL,SYSCALL_IO_WRITE_STRING,(uint64)string,0,0);
        letter++;
        for(uint64 i=0;i<50000*5000;i++)asm("NOP");
    }
}

void usercode2(){
    char letter='Z';
    while(1){
        if(letter<'A')letter='Z';
        asm volatile("outb %0, %1" : : "a" (letter), "Nd" (SERIAL_PORT));
        letter--;
        for(uint64 i=0;i<50000*5000;i++)asm("NOP");
    }
}

void kernel_panic(char* error){
    task_scheduler_running=0;
    asm("cli");
    print_to_serial("!!!Kernel panic!!!\n\r");
    print_to_serial("Error: ");
    print_to_serial(error);
    while1();
}