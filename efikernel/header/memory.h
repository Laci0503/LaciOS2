#ifndef _MEMORY_H
#define _MEMORY_H
#include <types.h>
#include <kernel_main.h>
#include <config.h>
#define EfiConventionalMemory 7
#define KERNEL_HEAP_SIZE_BYTES (KERNEL_HEAP_SIZE << 12)
#define END_HEAP_HEADER(header) ((heap_header*)((uint64)header+header->length+sizeof(heap_header))) // Gets the end header from the start header.
#define NEXT_HEAP_HEADER(header) ((heap_header*)((uint64)header+header->length+2*sizeof(heap_header))) // Gets the next start header from a start header.
#define SECONDARY_STACK_TOP (KERNEL_VMA_PDPT << 39) | (KERNEL_SECONDARY_STACK_START_PD << 30) + (KERNEL_SECONDARY_STACK_SIZE << 12) - 8

uint64* bitmap; //0: Free, 1: Resv
uint64 bitmap_length;
uint64 ram_amount; //in bytes
uint64 first_avail_idx;
uint64 usable_page_count;
uint64 first_avail_length;
uint64 io_next_page; //relative to kernel_start_page
uint64 kernel_start_page;

void init_memory_manager(kernel_info* kernel_info);
void* malloc_page(uint64 page_count);
void free_page(void* addr, uint64 page_count);
void map_system_tables();
void* map_page_to_kernel(void* address);
void* malloc(uint64 size);
void free(void* address);

void map_page(uint64 pdpt, uint64 pd, uint64 pt, uint64 page, uint64 phys_page, uint64**** pml4, uint64 flags);
void inc_pmap_vars(uint64* pdpt, uint64* pd, uint64* pt, uint64* page, uint64**** pml4);

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

#define HEAP_HEADER_ALLOCATED (1ULL<<63)
#define HEAP_HEADER_FRONT (1ULL<<62)

volatile typedef struct heap_header{
    uint64 allocated:1;
    uint64 length:63;
} heap_header;

page_map_level_4* kernel_pml4;

void* heap; //vma of the kernel heap
//heap_header* last_free_heap_header; // Pointer to the last heap header used by malloc

#endif