#include <task_scheduler.h>
#include <idt.h>
#include <memory.h>
#include <basic_functions.h>
#include <assembly_functions.h>

void init_task_scheduler(){
    task_switch_timer=0;
    for(uint32 i=0;i<sizeof(tasks_active);i++)((uint8*)tasks_active)[i]=0;
    tasks=(task*)malloc(sizeof(task)*MAX_TASK_NUM);
    task_count=0;
    task_switch_timer=1;
    cycle_per_task=5;
    current_task=-1;
}
void start_task_scheduler(){
    task_scheduler_running=1;
}
int64 add_task(void* location, uint64 size, uint64 entry_point) { //returns the index
    uint64 i;
    for(i=0;i<MAX_TASK_NUM+1;i++){
        if(i==MAX_TASK_NUM)return -1;
        if(!task_exists(i))break;
    }
    tasks[i].pml4=malloc_page(1);
    //for(uint32 j=0;j<512;j++)*(uint64*)(&(tasks[i]->pml4.page_directory_pointer_tables[j]))=0;
    tasks[i].pml4->page_directory_pointer_tables[4]=kernel_pml4->page_directory_pointer_tables[4];
    uint64 page_count=size/4096+6;
    uint64 pdpt_index=0;
    uint64 pd_index=0;
    uint64 pt_index=0;
    uint64 flags=0b111;
    uint64**** pml4=(uint64****)(tasks[i].pml4);

    pml4[pdpt_index]=(uint64***)((uint64)malloc_page(1) | flags);
    ((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)malloc_page(1) | flags;
    ((uint64*)((uint64)(((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)malloc_page(1) | flags);
    for(uint64 j=0;j<page_count;j++){
        void* pageframe=malloc_page(1);
        ((uint64*)(((uint64)((uint64*)((uint64)(((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]) & PAGE_ADDR_MASK))[j%512] = (uint64)pageframe | flags;
        for(uint32 k=0;k<512;k++)((uint64*)pageframe)[k]=((uint64*)(location+j*4096))[k];
        if((j+1)%512==0){
            pt_index++;
            if(pt_index!=512){
                ((uint64*)((uint64)(((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)malloc_page(1) | flags;
            }else{
                pd_index++;
                pt_index=0;
                if(pd_index!=512){
                    ((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)((uint64)malloc_page(1) | flags);
                    ((uint64*)((uint64)(((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)malloc_page(1) | flags);
                }
                else {
                    pdpt_index++;
                    pd_index=0;
                    pml4[pdpt_index]=(uint64***)((uint64)malloc_page(1) | flags);
                    ((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)malloc_page(1) | flags;
                    ((uint64*)((uint64)(((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)malloc_page(1) | flags);
                }
            }
        }
    }
    tasks[i].priority=1;
    tasks[i].privilage=0;
    tasks[i].state=PAUSED;
    tasks[i].id=i;
    tasks[i].cpu_state.cs=0x2b;
    tasks[i].cpu_state.ss=0x23;
    tasks[i].cpu_state.error_code=0;
    tasks[i].cpu_state.rsp=(page_count*4096)&(~0b111);
    tasks[i].cpu_state.rbp=tasks[i].cpu_state.rsp;
    tasks[i].cpu_state.rip=entry_point;
    tasks[i].cpu_state.rflags=0x202;//0x3202;
    tasks[i].cpu_state.rax=0;
    tasks[i].cpu_state.rbx=0;
    tasks[i].cpu_state.rcx=0;
    tasks[i].cpu_state.rdx=0;
    tasks[i].cpu_state.rdi=0;
    tasks[i].cpu_state.rsi=0;
    tasks[i].cpu_state.r8=0;
    tasks[i].cpu_state.r9=0;
    tasks[i].cpu_state.r10=0;
    tasks[i].cpu_state.r11=0;
    tasks[i].cpu_state.r12=0;
    tasks[i].cpu_state.r13=0;
    tasks[i].cpu_state.r14=0;
    tasks[i].cpu_state.r15=0;
    set_task_exists(i,1);
    task_count++;
    return i;
}

uint8 task_exists(uint64 idx){
    return (tasks_active[idx/64] & (1ULL<<(idx%64))) != 0;
}
void set_task_exists(uint64 idx, uint8 exists){
    if(exists){
        tasks_active[idx/64] |= (1ULL<<(idx%64));
    }else{
        tasks_active[idx/64] &= ~(1ULL<<(idx%64));
    }
}

uint8 schedule(uint64* current_pml4){
    //print_to_serial("scheduling\n\r");
    task_switch_timer--;
    *current_pml4=(uint64)tasks[current_task].pml4;
    if(task_switch_timer>0)return 0;
    task_switch_timer=1;
    uint64 i=0;
    if(current_task==-1)current_task=0;
    else{
        i=(current_task+1)%MAX_TASK_NUM;
        //print_to_serial("before while\n\r");
        while(!(task_exists(i) && tasks[i].state==RUNNING)){
            i++;
            i%=MAX_TASK_NUM;
        }
        tasks[current_task].cpu_state=current_state;
    }
    /*print_to_serial("Switchting to task #");
    print_int_to_serial(i);
    print_to_serial("\n\r");*/
    current_state=tasks[i].cpu_state;
    *current_pml4=(uint64)(tasks[i].pml4);
    task_switch_timer=tasks[i].priority*cycle_per_task;
    current_task=i;
    return 1;
}