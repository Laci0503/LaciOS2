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
#include "../../user_api/header/syscalls.h"

void usercode();
void usercode2();
uint8* kernel_stack_real_addr;
void* kernel_stack_vma;

void kernel_main(kernel_info* kernel_info){

    //print_to_serial("THIS IS KERNEL YEEEY!");

    framebuffer=(rgb*)kernel_info->frame_buffer;
    width=kernel_info->screen_width;
    height=kernel_info->screen_height;

    init_memory_manager(kernel_info);

    print_to_serial("sizeof(heap_header): ");
    print_hex_to_serial(sizeof(heap_header));
    print_to_serial("\n\r");

    void* t=malloc((KERNEL_HEAP_SIZE<<12) - 16);
    print_to_serial("t: ");
    print_hex_to_serial(t);
    print_to_serial("\n\r");
    free(t);
    print_to_serial("freed");

    void* a=malloc(15);
    print_to_serial("a: ");
    print_hex_to_serial(a);
    print_to_serial("\n\r");
    void* b=malloc(15);
    print_to_serial("b: ");
    print_hex_to_serial(b);
    print_to_serial("\n\r");
    void* c=malloc(15);
    print_to_serial("c: ");
    print_hex_to_serial(c);
    print_to_serial("\n\r");

    free(b);
    b=malloc(15);

    print_to_serial("b: ");
    print_hex_to_serial(b);
    print_to_serial("\n\r");

    /*gdt=kernel_info->gdt;
    tss=kernel_info->tss;
    init_memory_manager(kernel_info);
    print_to_serial("\n\rRam amount: \n\r");
    print_int_to_serial(ram_amount/(1024*1024));
    print_to_serial(" MB\n\r");
    print_to_serial("Usable page count: ");
    print_int_to_serial(usable_page_count);
    print_to_serial("; ");
    print_float_to_serial(((float80)usable_page_count*4096)/(1024*1024));
    print_to_serial(" MB\n\rBitmap address: ");
    print_hex_to_serial((uint64)bitmap);
    print_to_serial("\n\r");
    print_to_serial("First_avail address: ");
    print_hex_to_serial((uint64)&first_avail_idx);
    print_to_serial("\n\r");
    print_to_serial("First_avail_length address: ");
    print_hex_to_serial((uint64)&first_avail_length);
    print_to_serial("\n\r");
    print_to_serial("Kernel entry point virtual address: ");
    print_hex_to_serial((uint64)&kernel_main);
    print_to_serial("\n\r");
    print_to_serial("Tss (Kernel): ");
    print_hex_to_serial((uint64)tss);
    print_to_serial("\n\r");
    print_to_serial("Tss->rsp0: ");
    print_hex_to_serial(tss->rsp0);
    print_to_serial("\n\r");
    print_to_serial("Kernel pml4: ");
    print_hex_to_serial((uint64)kernel_pml4);
    print_to_serial("\n\r");
    print_to_serial("Screen width: ");
    print_int_to_serial(width);
    print_to_serial("\n\r");
    print_to_serial("Screen height: ");
    print_int_to_serial(height);
    print_to_serial("\n\r");
    print_to_serial("RSDP2 PTR: ");
    print_hex_to_serial(kernel_info->acpi_rsdp);
    print_to_serial("\n\r");
    uint64 rsdp2_address = kernel_info->acpi_rsdp;

    //Setting up kernel stack
    kernel_stack_real_addr=malloc_page(KERNEL_STACK_SIZE);
    for(uint32 i=0;i<KERNEL_STACK_SIZE;i++){
        kernel_stack_vma=(void*)((uint64)map_page_to_kernel(kernel_stack_real_addr + i*4096) + 4088);
    }

    uint64 kernel_secondary_stack_top=((KERNEL_VMA_PDPT << 39) | (KERNEL_SECONDARY_STACK_START_PD << 30)) + (KERNEL_SECONDARY_STACK_SIZE<<12) - 8;
    tss->rsp0=kernel_secondary_stack_top; //(uint64)&(kernel_stack_real_addr[5*4096])
    tss->rsp1=kernel_secondary_stack_top; //(uint64)&(kernel_stack_real_addr[5*4096])
    tss->rsp2=kernel_secondary_stack_top; //(uint64)&(kernel_stack_real_addr[5*4096])

    init_idt();
    enable_sce();
    print_to_serial("&usercode: ");
    print_hex_to_serial((uint64)usercode);
    print_to_serial("\n\rTss: ");
    print_hex_to_serial((uint64)tss);
    print_to_serial("\n\rGdt: ");
    print_hex_to_serial((uint64)gdt);
    print_to_serial("\n\r");

    print_to_serial("Kernel start page: ");
    print_hex_to_serial(kernel_start_page);
    print_to_serial("\n\r");
    print_to_serial("Kernel next page: ");
    print_hex_to_serial(kernel_next_page);
    print_to_serial("\n\r");

    map_system_tables();

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