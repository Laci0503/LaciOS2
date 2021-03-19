#ifndef _TASK_SCHEDULER_H
#define _TASK_SCHEDULER_H
#include <types.h>
#include <idt.h>
#include <memory.h>

#define MAX_TASK_NUM 64

typedef enum task_state{
    RUNNING,
    PAUSED,
    FROSEN,
    TERMINATED
} task_state;

typedef struct task{
    page_map_level_4* pml4;
    CPU_state cpu_state;
    uint8 name[32];
    uint64 id;
    uint64 priority;
    uint64 privilage; //later maybe
    task_state state;
} task;

task* tasks;
uint64 tasks_active[MAX_TASK_NUM/64]; //bitmap
uint64 task_count;
int64 current_task;
uint64 task_switch_timer;
uint64 cycle_per_task;
uint8 task_scheduler_running;

void init_task_scheduler();
uint8 task_exists(uint64 idx);
int64 add_task(void* location, uint64 size, uint64 entry_point);
uint8 schedule(uint64* current_pml4);
void start_task_scheduler();
void set_task_exists(uint64 idx, uint8 exists);

#endif