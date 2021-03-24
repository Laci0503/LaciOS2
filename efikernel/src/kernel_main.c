#include <types.h>
#include <kernel_main.h>
#include <basic_functions.h>
#include <video.h>
#include <assembly_functions.h>
#include <memory.h>
#include <gdt.h>
#include <idt.h>
#include <task_scheduler.h>

void usercode();
void usercode2();
uint8 user_stack[2048];
uint8 test[2048];
uint8* kernel_stack_real_addr;
void* kernel_stack_vma;

void kernel_main(kernel_info* kernel_info){
    framebuffer=(rgb*)kernel_info->frame_buffer;
    width=kernel_info->screen_width;
    height=kernel_info->screen_height;

    task_scheduler_running=0;
    gdt=kernel_info->gdt;
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

    /*void* a=malloc(1);

    void* b=malloc(1);
    void* c=malloc(1);

    print_to_serial("a: ");
    print_hex_to_serial(a);
    print_to_serial("\n\r");
    print_to_serial("b: ");
    print_hex_to_serial(b);
    print_to_serial("\n\r");
    print_to_serial("c: ");
    print_hex_to_serial(c);
    print_to_serial("\n\r");
    print_to_serial("First_avail_idx: ");
    print_hex_to_serial(first_avail_idx);
    print_to_serial("\n\r");
    print_to_serial("First_avail_length: ");
    print_hex_to_serial(first_avail_length);
    print_to_serial("\n\r");

    free(c,1);
    print_to_serial("free(b,1) First_avail_idx: ");
    print_hex_to_serial(first_avail_idx);
    print_to_serial("\n\r");
    print_to_serial("First_avail_length: ");
    print_hex_to_serial(first_avail_length);
    print_to_serial("\n\r");
    free(b,1);
    print_to_serial("free(c,1) First_avail_idx: ");
    print_hex_to_serial(first_avail_idx);
    print_to_serial("\n\r");
    print_to_serial("First_avail_length: ");
    print_hex_to_serial(first_avail_length);
    print_to_serial("\n\r");
    b=malloc(1);
    c=malloc(1);

    print_to_serial("a: ");
    print_hex_to_serial(a);
    print_to_serial("\n\r");
    print_to_serial("b: ");
    print_hex_to_serial(b);
    print_to_serial("\n\r");
    print_to_serial("c: ");
    print_hex_to_serial(c);
    print_to_serial("\n\r");
    print_to_serial("First_avail_idx: ");
    print_hex_to_serial(first_avail_idx);
    print_to_serial("\n\r");
    print_to_serial("First_avail_length: ");
    print_hex_to_serial(first_avail_length);
    print_to_serial("\n\r");*/

    //print_to_serial("GDT_code segment: ");
    //print_int_to_serial((uint64)&(gdt->kernel_code)-(uint64)gdt);

    kernel_stack_real_addr=malloc(KERNEL_STACK_SIZE);
    for(uint32 i=0;i<KERNEL_STACK_SIZE;i++){
        kernel_stack_vma=(void*)((uint64)map_page_to_kernel(kernel_stack_real_addr + i*4096) + 4088);
    }

    tss->rsp0=(uint64)kernel_stack_vma; //(uint64)&(kernel_stack_real_addr[5*4096])
    tss->rsp1=(uint64)kernel_stack_vma; //(uint64)&(kernel_stack_real_addr[5*4096])
    tss->rsp2=(uint64)kernel_stack_vma; //(uint64)&(kernel_stack_real_addr[5*4096])

    init_idt();
    enable_sce();
    print_to_serial("&usercode: ");
    print_hex_to_serial((uint64)usercode);
    print_to_serial("\n\r&user_stack[1023]: ");
    print_hex_to_serial((uint64)&user_stack[1024]);
    print_to_serial("\n\rTss: ");
    print_hex_to_serial((uint64)tss);
    print_to_serial("\n\rGdt: ");
    print_hex_to_serial((uint64)gdt);
    print_to_serial("\n\r");
    //asm("cli");
    //while1();
    //enter_userspace((uint64)usercode,(uint64)&(user_stack[1023]),0x202);
    /*char letter='A';
    while(1){
        if(letter>'Z')letter='A';
        outb(SERIAL_PORT,letter);
        letter++;
        for(uint32 i=0;i<5000;i++){
            for(uint32 j=0;j<50000;j++){
                asm("NOP");
            }
        }
    }*/
    /*uint64 a=0

    /*char letter='A';
    while(1){
        if(letter>'Z')letter='A';
        outb(SERIAL_PORT,letter);
        letter++;
        for(uint32 i=0;i<5000;i++){
            for(uint32 j=0;j<50000;j++){
                asm("NOP");
            }
        }
    }*/

    //add_task(usercode,sizeof(usercode),0);
    //print_to_serial("sizeof(usercode): ");
    //print_int_to_serial(sizeof(usercode));
    //print_to_serial("\n\r");

    //enter_userspace((uint64)usercode,(uint64)(&user_stack[1023]),0x202);

    //tasks[idx].state=RUNNING;*/
    //init_task_scheduler();

    print_to_serial("Kernel start page: ");
    print_hex_to_serial(kernel_start_page);
    print_to_serial("\n\r");
    print_to_serial("Kernel next page: ");
    print_hex_to_serial(kernel_next_page);
    print_to_serial("\n\r");

    map_system_tables();

    init_task_scheduler();
    int64 idx=add_task(usercode2,4096,0);
    tasks[idx].state=RUNNING;
    print_to_serial("Tasks: ");
    print_hex_to_serial((uint64)tasks);
    print_to_serial("\n\r");
    print_to_serial("idx: ");
    print_signed_to_serial(idx);
    print_to_serial("\n\r");
    print_to_serial("tasks[1].pml4: ");
    print_hex_to_serial((uint64)(tasks[idx].pml4));
    print_to_serial("\n\r");
    print_to_serial("&current_state: ");
    print_hex_to_serial((uint64)(&current_state));
    print_to_serial("\n\r");

    idx=add_task(usercode,4096,0);
    tasks[idx].state=RUNNING;

    idx=add_task(usercode,4096,0);
    tasks[idx].state=RUNNING;

    idx=add_task(usercode,4096,0);
    tasks[idx].state=RUNNING;

    idx=add_task(usercode,4096,0);
    tasks[idx].state=RUNNING;
    
    idx=add_task(usercode,4096,0);
    tasks[idx].state=RUNNING;

    //print_to_serial("tasks[2].pml4: ");
    //print_hex_to_serial((uint64)(tasks[idx].pml4));
    //print_to_serial("\n\r");

    start_task_scheduler();

    while(1);
}

void usercode(){
    char letter='A';
    while(1){
        if(letter>'Z')letter='A';
        asm volatile("outb %0, %1" : : "a" (letter), "Nd" (SERIAL_PORT));
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