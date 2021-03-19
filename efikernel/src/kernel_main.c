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
uint8 user_stack[2048];
uint8 test[2048];
uint8* kernel_stack;

void kernel_main(kernel_info* kernel_info){
    print_to_serial("kernel!!");
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

    kernel_stack=malloc(5);

    tss->rsp0=(uint64)&(kernel_stack[5*4096]);
    tss->rsp1=(uint64)&(kernel_stack[5*4096]);
    tss->rsp2=(uint64)&(kernel_stack[5*4096]);
    /*tss->rsp1=0xffffffffffffffff;//(uint64)&(test[1024]);
    tss->rsp2=0xffffffffffffffff;//(uint64)&(test[1024]);
    tss->ist1=0xffffffffffffffff;//(uint64)&(test[1024]);*/

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
    int64 idx=add_task(usercode,4096,0);
    tasks[idx].state=RUNNING;
    print_to_serial("Tasks: ");
    print_hex_to_serial((uint64)tasks);
    print_to_serial("\n\r");
    print_to_serial("idx: ");
    print_signed_to_serial(idx);
    print_to_serial("\n\r");
    print_to_serial("tasks[i].pml4->pdptt[4]: ");
    print_hex_to_serial((uint64)(tasks[idx].pml4));
    print_to_serial("\n\r");
    print_to_serial("&current_state: ");
    print_hex_to_serial((uint64)(&current_state));
    print_to_serial("\n\r");
    start_task_scheduler();

    while(1);
}
void usercode(){
    /*print_to_serial("\n\rInthandler address: ");
    print_hex_to_serial(((uint64)(IDT[0x20].offset_1)) | (((uint64)IDT[0x20].offset_2)<<16) | (((uint64)IDT[0x20].offset_3)<<32));*/
    /*print_hex_to_serial(IDT[0x21].offset_1);
    print_to_serial("\n\rOffset_2: ");
    print_hex_to_serial(IDT[0x21].offset_2);
    print_to_serial("\n\rOffset_3: ");
    print_hex_to_serial(IDT[0x21].offset_3);*/
    //while(1);
    char letter='A';
    while(1){
        if(letter>'Z')letter='A';
        outb(SERIAL_PORT,letter);
        letter++;
        /*for(uint32 i=0;i<5000;i++){
            for(uint32 j=0;j<50000;j++){
                asm("NOP");
            }
        }*/
    }

}