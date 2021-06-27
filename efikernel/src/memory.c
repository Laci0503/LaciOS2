#include <memory.h>
#include <kernel_main.h>
#include <basic_functions.h>
#include <gdt.h>
#include <assembly_functions.h>

void init_memory_manager(kernel_info* kernel_info){
    EFI_MEMORY_DESCRIPTOR* memmap=kernel_info->memmap;
    uint64 memmap_desc_count=kernel_info->memmap_desc_count;
    uint64 memmap_desc_size=kernel_info->memmap_desc_size;
    uint64 main_part_addr=((EFI_MEMORY_DESCRIPTOR*)(((uint64)memmap)+((kernel_info->largest_area_idx)*memmap_desc_size)))->PhysicalStart;
    EFI_MEMORY_DESCRIPTOR* last_desc=(EFI_MEMORY_DESCRIPTOR*)(((uint64)memmap)+memmap_desc_size*(memmap_desc_count-1));
    /*print_to_serial("\n\rMemory main part addr: ");
    print_hex_to_serial((uint64)main_part_addr);
    print_to_serial("\n\rNumber of used pages: ");
    print_int_to_serial(kernel_info->used_pages);
    print_to_serial("\n\r");*/
    ram_amount=last_desc->PhysicalStart+last_desc->NumberOfPages*4096;
    uint64 pageframe_count=ram_amount/4096;
    bitmap=(uint64*)(main_part_addr+kernel_info->used_pages*4096);
    bitmap_length=pageframe_count/(4096*8);
    kernel_info->used_pages+=bitmap_length;
    /*print_to_serial("Bitmap location: ");
    print_hex_to_serial(bitmap);
    print_to_serial("\n\r");
    print_to_serial("Kernel location: ");
    print_hex_to_serial(kernel_info->real_address);
    print_to_serial("\n\r");*/

    for(uint64 i=0;i<bitmap_length*512;i++){
        bitmap[i]=~0ULL;
    }

    usable_page_count=0;
    #if(MEMORY_DEBUG)
        print_to_serial("Memmap: \n\r");
    #endif
    for(uint64 i=0;i<memmap_desc_count;i++){
        EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*)((uint64)memmap+i*memmap_desc_size);
        #if(MEMORY_DEBUG)
            print_to_serial("idx: ");
            print_int_to_serial(i);
            print_to_serial("; addr: ");
            print_hex_to_serial(descriptor->PhysicalStart);
            print_to_serial("; length: ");
            print_int_to_serial(descriptor->NumberOfPages);
            print_to_serial("; type: ");
            print_int_to_serial(descriptor->Type);
        #endif
        if(descriptor->Type==EfiConventionalMemory){
            uint64 pageframe=descriptor->PhysicalStart/4096;
            usable_page_count+= descriptor->NumberOfPages;
            for(uint64 i=pageframe;i<pageframe+descriptor->NumberOfPages;i++){
                bitmap[(i)/64] &= ~(0x1ULL << (i%64));
            }
            #if(MEMORY_DEBUG)
                print_to_serial(" Added");
            #endif
        }
        #if(MEMORY_DEBUG)
            print_to_serial("\n\r");
        #endif
    }
    for(uint64 i=main_part_addr/4096;i<main_part_addr/4096+kernel_info->used_pages;i++){
        bitmap[(i)/64] |= (0x1ULL << (i%64));
    }
    uint64 kernel_pml4_addr=kernel_info->kernel_pml4_addresss;
    bitmap[kernel_pml4_addr/64] |= (1ULL<<(kernel_pml4_addr%64));
    bitmap[((uint64)(kernel_info->gdt))/64] |= (1ULL<<(((uint64)(kernel_info->gdt))%64));
    bitmap[((uint64)(kernel_info->tss))/64] |= (1ULL<<(((uint64)(kernel_info->tss))%64));
    first_avail_idx=0;
    first_avail_length=1;
    //while(bitmap[(first_avail_idx)/64] & (0x1 << first_avail_idx%64) != 0)first_avail_idx++;
    while((bitmap[first_avail_idx/64] & (0x1ULL << (first_avail_idx % 64))) != 0)first_avail_idx++;
    while((bitmap[(first_avail_idx+first_avail_length)/64] & (0x1ULL << ((first_avail_idx+first_avail_length)%64))) == 0)first_avail_length++;
    kernel_pml4=(page_map_level_4*)kernel_pml4_addr;

    kernel_start_page=((uint64)kernel_main)>>12;
    kernel_next_page=kernel_info->kernel_next_page;

    heap=(void*)((KERNEL_VMA_PDPT << 39) | (KERNEL_HEAP_START_PD << 30));
    last_free_heap_header=(heap_header*)heap;
    last_free_heap_header->length=(KERNEL_HEAP_SIZE<<12) - (sizeof(heap_header)*2);
    last_free_heap_header->front=1;
    last_free_heap_header->allocated=0;
    
    ((heap_header*)((uint64)heap+KERNEL_HEAP_SIZE-sizeof(heap_header)))->length=(KERNEL_HEAP_SIZE<<12) - (sizeof(heap_header)*2);
    ((heap_header*)((uint64)heap+KERNEL_HEAP_SIZE-sizeof(heap_header)))->front=0;
    ((heap_header*)((uint64)heap+KERNEL_HEAP_SIZE-sizeof(heap_header)))->allocated=0;

    #if(MEMORY_DEBUG)
        print_to_serial("last_free_heap_header: ");
        print_hex_to_serial((uint64)last_free_heap_header);
        print_to_serial("\n\r");
    #endif
}

//TODO: Nincs kezelve ha nem tud annyit lefoglalni
//TODO: Csak akkor működik ha a kernel_pml4 van használatban, nem ellenőrzi

void* malloc_page(uint64 page_count){ //TODO: Lehetne még optimalizálni
    uint64 retval;
    if(page_count<first_avail_length){ //kisebb mint az első
        retval=first_avail_idx*4096;
        first_avail_idx+=page_count;
        first_avail_length-=page_count;
        for(uint64 i=first_avail_idx;i<first_avail_idx+page_count;i++)bitmap[(i)/64] |= (0x1ULL << (i%64));
    }else if(first_avail_length==page_count){ // pont annyi mint az első
        retval=first_avail_idx*4096;
        for(uint64 i=first_avail_idx;i<first_avail_idx+page_count;i++)bitmap[(i)/64] |= (0x1ULL << (i%64));
        first_avail_idx+=page_count+1;
        while((bitmap[first_avail_idx/64] & (0x1ULL << (first_avail_idx % 64))) != 0)first_avail_idx++;
        first_avail_length=1;
        while((bitmap[(first_avail_idx+first_avail_length)/64] & (0x1ULL << ((first_avail_idx+first_avail_length)%64))) == 0)first_avail_length++;
    }else{ // több mint az első
        retval=first_avail_idx+first_avail_length+1;
        uint64 l=0;
        while(l<page_count){
            retval+=l;
            while((bitmap[retval/64] & (0x1ULL << (retval%64))) != 0)retval++;
            l=0;
            while(((bitmap[(retval+l)/64] & (0x1ULL << ((retval+l)%64))) == 0) && (l<page_count))l++;
        }
        for(uint64 i=retval;i<retval+page_count;i++)bitmap[(i)/64] |= (0x1ULL << (i%64));
        
        retval <<= 12;
    }
    for(uint64 i=retval;i<page_count*512;i++)*(uint64*)i=0;
    return (void*)retval;
}

void free_page(void* addr, uint64 page_count){
    uint64 startpage=(uint64)addr >> 12;
    /*print_to_serial("Startpage: ");
    print_int_to_serial(startpage);*/
    
    for(uint64 i=startpage;i<startpage+page_count;i++)bitmap[(i)/64] &= ~(0x1ULL << (i%64)); //TODO: Lehetne még optimalizálni
    if(startpage==(first_avail_idx+first_avail_length)){ // közvetlenül az első után
        first_avail_length+=page_count;
    }else if(first_avail_idx==(startpage+page_count)){ // közvetlenül az első előtt
        first_avail_length+=page_count;
        first_avail_idx=startpage;
    }else if(startpage<first_avail_idx) { // első előtt
        
        //print_to_serial(" Első előtt first_avail: ");
        first_avail_idx=startpage;
        //print_int_to_serial(first_avail_idx);
        while(((bitmap[(first_avail_idx-1)/64] & (0x1ULL << ((first_avail_idx-1)%64))) == 0) && ((first_avail_idx-1)!=0))first_avail_idx--;
        //print_to_serial(";After first while: first_avail: ");
        //print_int_to_serial(first_avail_idx);
        first_avail_length=(startpage-first_avail_idx)+page_count;
        while((bitmap[(first_avail_idx+first_avail_length)/64] & (0x1ULL << ((first_avail_idx+first_avail_length)%64))) == 0)first_avail_length++;
    } // első után; nem kell semmit csinálni
    //print_to_serial("\n\r");
}

void map_system_tables(){

    uint64 mapped_tss_addr=(uint64)map_page_to_kernel(tss);
    uint64 mapped_gdt_addr=(uint64)map_page_to_kernel(gdt);

    gdt->tss_low.base15_0 = mapped_tss_addr & 0xffff;
    gdt->tss_low.base23_16 = (mapped_tss_addr >> 16) & 0xff;
    gdt->tss_low.base31_24 = (mapped_tss_addr >> 24) & 0xff;
    gdt->tss_low.limit15_0 = sizeof(tss);
    gdt->tss_high.limit15_0 = (mapped_tss_addr >> 32) & 0xffff;
    gdt->tss_high.base15_0 = (mapped_tss_addr >> 48) & 0xffff;

    gdt_ptr ptr;
    ptr.base=mapped_gdt_addr;
    ptr.limit=sizeof(*gdt)-1;
    print_to_serial("TSS address: ");
    print_hex_to_serial(mapped_tss_addr);
    print_to_serial("\n\r");
    print_to_serial("GDT address: ");
    print_hex_to_serial(mapped_gdt_addr);
    print_to_serial("\n\r");
    print_to_serial("&ptr: ");
    print_hex_to_serial((uint64)&ptr);
    print_to_serial("\n\r");
    //while1();
    load_gdt(&ptr);
}

void* map_page_to_kernel(void* address){
    uint64 kernel_addr=(kernel_start_page+kernel_next_page)<<12;
    uint64 pdpt_index=kernel_addr >> 39;
    uint64 pd_index=(kernel_addr >> 30) & 511;
    uint64 pt_index=(kernel_addr >> 21) & 511;
    uint64 j=(kernel_addr >> 12) & 511;
    uint64**** pml4=(uint64****)(kernel_pml4);
    uint64 flags=0b111;
    ((uint64*)(((uint64)((uint64*)((uint64)(((uint64*)((uint64)pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]) & PAGE_ADDR_MASK))[j%512] = ((uint64)address & (~0xfff)) | flags;
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
    kernel_next_page++;
    return (void*)kernel_addr;
}

void* malloc(uint64 size){
    
}

void free(void* address){
    
}

/*void* malloc(uint64 size){
    if(last_free_heap_header!=NULL){ // If-ek sorrendjével lehet még optimalizálni (kivenni az &&-t)
        #if(MEMORY_DEBUG)
            print_to_serial("last not null");
        #endif
        if(last_free_heap_header->length>=size && last_free_heap_header->length < size+sizeof(heap_header)*2){
            last_free_heap_header->allocated=1;
            ((heap_header*)((uint64)last_free_heap_header+last_free_heap_header->length+sizeof(heap_header)))->allocated=1;
            void* return_addr=(void*)((uint64)last_free_heap_header+sizeof(heap_header));
            last_free_heap_header=NULL;
            return return_addr;
        }else if (last_free_heap_header->length>size+sizeof(heap_header)*2){
            #if(MEMORY_DEBUG)
                print_to_serial("last_free_heap_header: ");
                print_hex_to_serial((uint64)last_free_heap_header);
                print_to_serial("\n\r");
            #endif
            uint64 free_header_length=last_free_heap_header->length;
            heap_header* front_header=last_free_heap_header;
            front_header->allocated=1;
            front_header->length=size;
            heap_header* end_header=(heap_header*)((uint64)front_header+sizeof(heap_header)+size);
            end_header->length=size;
            end_header->allocated=1;
            end_header->front=0;
            heap_header* free_front=(heap_header*)((uint64)end_header+sizeof(heap_header));
            free_front->allocated=0;
            free_front->front=1;
            free_front->length=free_header_length - sizeof(heap_header)*2 - size;
            last_free_heap_header=free_front;
            heap_header* free_end=(heap_header*)((uint64)front_header + sizeof(heap_header)+free_header_length);
            free_end->length=free_front->length;
            return (void*)((uint64)front_header + sizeof(heap_header));
        }
    }
    heap_header* header = heap;
    while(!(header->allocated==0 && header->length>=size)){
        if((uint64)header + header->length + sizeof(heap_header)*2 >= (uint64)heap + KERNEL_HEAP_SIZE << 12)return NULL;
        header = (heap_header*)((uint64)header + header->length + sizeof(heap_header)*2);
    }

    if(header->length>=size && header->length < size+sizeof(heap_header)*2){
        header->allocated=1; // Flag allocated
        ((heap_header*)((uint64)header+header->length+sizeof(heap_header)))->allocated=1; // Flag end header allocated
        return (void*)(header+sizeof(heap_header));
    }else if (header->length>size+sizeof(heap_header)*2){
        uint64 free_header_length=header->length;
        heap_header* front_header=header;
        front_header->allocated=1;
        front_header->length=size;
        heap_header* end_header=(heap_header*)((uint64)front_header+sizeof(heap_header)+size);
        end_header->length=size;
        end_header->allocated=1;
        end_header->front=0;
        heap_header* free_front=(heap_header*)((uint64)end_header+sizeof(heap_header));
        free_front->allocated=0;
        free_front->front=1;
        free_front->length=free_header_length - sizeof(heap_header)*2 - size;
        heap_header* free_end=(heap_header*)((uint64)front_header + sizeof(heap_header)+free_header_length);
        free_end->length=free_front->length;
        return (void*)((uint64)front_header + sizeof(heap_header));
    }

}

void free(void* address){
    if((uint64)address<(uint64)heap || (uint64)address > (uint64)heap + KERNEL_HEAP_SIZE << 12){
        #if(MEMORY_DEBUG)
            // Some error message maybe ¯\_(ツ)_/¯
        #endif
        return;
    }
    heap_header* header = (heap_header*)((uint64)address - sizeof(heap_header));
    heap_header* end_header = (heap_header*)((uint64)header + header->length + sizeof(heap_header));
    header->allocated=0;
    end_header->allocated=0;
    heap_header* before_header=(heap_header*)((uint64)header - sizeof(heap_header));
    if((uint64)before_header > (uint64)heap)
    if(before_header->allocated==0){
        uint64 new_length=before_header->length + header->length + sizeof(heap_header)*2;
        heap_header* before_start_header=(heap_header*)((uint64)before_header - before_header->length - sizeof(heap_header));
        before_start_header->length=new_length;
        end_header->length=new_length;
        header=before_start_header;
    }
    heap_header* next_header = (heap_header*)((uint64)end_header+sizeof(heap_header));
    if((uint64)next_header < (uint64)heap + KERNEL_HEAP_SIZE << 12)
    if(next_header->allocated==0){
        uint64 new_length = header->length + next_header->length + sizeof(heap_header)*2;
        heap_header* next_end_header = (heap_header*)((uint64)next_header + next_header->length + sizeof(heap_header));
        next_end_header->length=new_length;
        header->length=new_length;
    }
}*/