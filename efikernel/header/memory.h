#ifndef _MEMORY_H
#define _MEMORY_H
#include <types.h>
#include <kernel_main.h>
#define EfiConventionalMemory 7
#define PAGE_ADDR_MASK ~0xFFFULL

uint64* bitmap; //0: Free, 1: Resv
uint64 bitmap_length;
uint64 ram_amount; //in bytes
uint64 first_avail_idx;
uint64 usable_page_count;
uint64 first_avail_length;
uint64 kernel_next_page; //relative to kernel_start_page
uint64 kernel_start_page;

void init_memory_manager(kernel_info* kernel_info);
void* malloc(uint64 page_count);
void free(void* addr, uint64 page_count);
void map_system_tables();
void* map_page_to_kernel(void* address);


volatile typedef struct page_table{
    struct page_pointer{
        uint64 present:1;
        uint64 writeable:1;
        uint64 user:1;
        uint64 write_through_cache:1;
        uint64 cache_disable:1;
        uint64 accessed:1;
        uint64 dirty:1;
        uint64 resv:1;
        uint64 global:1;
        uint64 available:3;
        uint64 pageframe_idx:52;
    } pages[512];
} page_table;

volatile typedef struct page_directory{
    struct page_table_pointer{
        uint64 present:1;
        uint64 writeable:1;
        uint64 user:1;
        uint64 write_through_cache:1;
        uint64 cache_disable:1;
        uint64 accessed:1;
        uint64 resv:1;
        uint64 size:1;
        uint64 ignored:1;
        uint64 available:3;
        uint64 pageframe_idx:52;
    } page_tables[512];
} page_directory;

volatile typedef struct page_directory_pointer_table{
    struct page_directory_pointer{
        uint64 present:1;
        uint64 writeable:1;
        uint64 user:1;
        uint64 write_through_cache:1;
        uint64 cache_disable:1;
        uint64 accessed:1;
        uint64 resv:1;
        uint64 size:1;
        uint64 ignored:1;
        uint64 available:3;
        uint64 pageframe_idx:52;
    } page_directories[512];
} page_directory_pointer_table;

volatile typedef struct page_map_level_4{
    struct page_directory_pointer_table_pointer{
        uint64 present:1;
        uint64 writeable:1;
        uint64 user:1;
        uint64 write_through_cache:1;
        uint64 cache_disable:1;
        uint64 accessed:1;
        uint64 resv:1;
        uint64 size:1;
        uint64 ignored:1;
        uint64 available:3;
        uint64 pageframe_idx:52;
    } page_directory_pointer_tables[512];
} page_map_level_4;

page_map_level_4* kernel_pml4;

#endif