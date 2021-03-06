#include <idt.h>
#include <assembly_functions.h>
#include <basic_functions.h>
#include <gdt.h>
#include <kernel_main.h>
#include <task_scheduler.h>

#if (IDT_DEBUG)
    uint64 test_idt;
#endif

void init_idt(){
    uint32 inthandler_size=(uint64)interrupts_int1-(uint64)interrupts_int0;
    uint64 first_int_handler_addr=(uint64)interrupts_int0;
    #if(IDT_DEBUG)
        print_to_serial("Inthandlers: ");
        print_hex_to_serial(first_int_handler_addr);
        print_to_serial("\r\nIDT: ");
        print_hex_to_serial((uint64)IDT);
        print_to_serial("\n\r");
    #endif

    for(uint32 i=0;i<256;i++){
        uint64 handler_addr=first_int_handler_addr+inthandler_size*i;
        IDT[i].offset_1=handler_addr & 0xffff;
        IDT[i].offset_2=(handler_addr >> 16) & 0xffff;
        IDT[i].offset_3=(handler_addr >> 32) & 0xffffffff;
        IDT[i].resv=0;
        IDT[i].selector = 8;//(uint64)&(gdt->kernel_code)-(uint64)gdt;
        IDT[i].type_attr=0b11101110;
        IDT[i].interrupt_stack_table_selector=0;
    }

    //Div by zero excep.
    //IDT[0].type_attr |= 0xf;

    for(uint32 i=0;i<32;i++)IDT[i].type_attr |= 0x1;
    IDT[0x2].type_attr &= 0xfe;

    #if(IDT_DEBUG)
        print_to_serial("Done constructing the IDT.\n\r");
    #endif

    idt_descriptor IDT_DESC;
    IDT_DESC.base=(uint64)IDT;
    IDT_DESC.limit=256*sizeof(idt_entry)-1;
    asm("cli");
    asm volatile("lidt %0" : : "m" (IDT_DESC));

    outb(PICMasterCommand,0x11);
    outb(PICSlaveCommand,0x11);

    outb(PICMasterData,HardwareInterruptOffset);
    outb(PICSlaveData,HardwareInterruptOffset);

    outb(PICMasterData,0x4);
    outb(PICSlaveData,0x2);

    outb(PICMasterData,0x1);
    outb(PICSlaveData,0x1);

    outb(PICMasterData,0x0);
    outb(PICSlaveData,0x0);
    task_scheduler_running=0;

    asm("sti");

    #if(IDT_DEBUG)
        test_idt=0;
        print_to_serial("IDT setup complete. \n\r");
    #endif
}

void handleInterrupt(uint64 interruptnumber){
    #if(IDT_DEBUG)
        print_to_serial("int;#");
        print_int_to_serial(test_idt);
        print_to_serial("\n\r");
        test_idt++;
    #endif

    uint64 current_pml4;
    if(interruptnumber==HardwareInterruptOffset+0){
        if(task_scheduler_running){
            /*//asm("mov %%rax, %%cr3" : : "a"(kernel_pml4));
            load_pml4(kernel_pml4);
            schedule(&current_pml4);
            #if(IDT_DEBUG)
                print_to_serial("Setting pml4:");
                print_hex_to_serial(current_pml4);
            #endif
            //asm("mov %%rax, %%cr3" : : "a"(current_pml4));
            load_pml4((page_map_level_4*)current_pml4);
            #if(IDT_DEBUG)
                print_to_serial(" Set!;#");
                print_int_to_serial(test_idt);
                print_to_serial("\n\r");
            #endif*/
        }
    }
    //print_to_serial("Interrupt number: ");
    //print_int_to_serial(interruptnumber);
    //print_to_serial("\n\r");
    if(interruptnumber<0x20){
        print_to_serial("!!! Exception: ");
        print_hex_to_serial(interruptnumber);
        print_to_serial("\n\rError code: ");
        print_hex_to_serial(current_state.error_code);

        print_to_serial("\n\rSS: ");
        print_hex_to_serial(current_state.ss);
        print_to_serial("\n\r");
        print_to_serial("CS: ");
        print_hex_to_serial(current_state.cs);
        print_to_serial("\n\r");
        print_to_serial("RSP: ");
        print_hex_to_serial(current_state.rsp);
        print_to_serial("\n\r");
        print_to_serial("RAX: ");
        print_hex_to_serial(current_state.rax);
        print_to_serial("\n\r");
        print_to_serial("RIP: ");
        print_hex_to_serial(current_state.rip);
        print_to_serial("\n\r");
        print_to_serial("instr: ");
        for(uint32 i=0;i<15;i++){
            print_hex_to_serial(*(uint8*)(current_state.rip+i));
            print_to_serial(" ");
        }
        print_to_serial("\n\r!!!");
        #if(IDT_DEBUG)
            print_to_serial(";#");
            print_int_to_serial(test_idt);
            print_to_serial("\n\r");
        #endif
        //while1();
    }
    //print_to_serial("1\n\r");
    if(interruptnumber==HardwareInterruptOffset+1){
        inb(KEYBOARD_PORT);
    }
    if(interruptnumber>=HardwareInterruptOffset && interruptnumber<=HardwareInterruptOffset+0x10){
        outb(PICMasterCommand,0x20);
        if(interruptnumber>=HardwareInterruptOffset+8)outb(PICSlaveCommand,0x20);
    }
    #if(IDT_DEBUG)
        print_to_serial("returning");
        print_hex_to_serial(current_pml4);
        print_to_serial("\n\r");
    #endif
}