#include <memory.h>
#include <kernel_main.h>
#include <basic_functions.h>
#include <gdt.h>
#include <assembly_functions.h>

void init_memory_manager(kernel_info* kernel_info){
    gdt=kernel_info->gdt;
    tss=kernel_info->tss;
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
    bitmap[kernel_pml4_addr/64] |= (1ULL<<(kernel_pml4_addr%64)); // Mark the kernel pml4 as reserved
    bitmap[((uint64)(kernel_info->gdt))/64] |= (1ULL<<(((uint64)(kernel_info->gdt))%64)); // Mark the gdt as reserved
    bitmap[((uint64)(kernel_info->tss))/64] |= (1ULL<<(((uint64)(kernel_info->tss))%64)); // Mark the tss as reserved
    first_avail_idx=0;
    first_avail_length=1;
    //while(bitmap[(first_avail_idx)/64] & (0x1 << first_avail_idx%64) != 0)first_avail_idx++;
    while((bitmap[first_avail_idx/64] & (0x1ULL << (first_avail_idx % 64))) != 0)first_avail_idx++;
    while((bitmap[(first_avail_idx+first_avail_length)/64] & (0x1ULL << ((first_avail_idx+first_avail_length)%64))) == 0)first_avail_length++;
    kernel_pml4=(page_map_level_4*)kernel_pml4_addr;

    kernel_start_page=((uint64)kernel_main)>>12;

    heap=(void*)((KERNEL_VMA_PDPT << 39) | (KERNEL_HEAP_START_PD << 30));
    heap_header* header;
    header=(heap_header*)heap;
    header->length=(KERNEL_HEAP_SIZE<<12) - (sizeof(heap_header)*2);
    header->allocated=0;
    
    ((heap_header*)((uint64)heap+KERNEL_HEAP_SIZE-sizeof(heap_header)))->length=(KERNEL_HEAP_SIZE<<12) - (sizeof(heap_header)*2);
    ((heap_header*)((uint64)heap+KERNEL_HEAP_SIZE-sizeof(heap_header)))->allocated=0;

    #if(MEMORY_DEBUG)
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
        print_to_serial("First_avail_idx: ");
        print_hex_to_serial(first_avail_idx);
        print_to_serial("\n\r");
        print_to_serial("First_avail_length: ");
        print_hex_to_serial(first_avail_length);
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
    #endif

    io_next_page=kernel_info->io_space_used_pages;

    map_system_tables();
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

    tss->rsp0 = SECONDARY_STACK_TOP;
    tss->rsp1 = SECONDARY_STACK_TOP;
    tss->rsp2 = SECONDARY_STACK_TOP;

    gdt_ptr ptr;
    ptr.base=mapped_gdt_addr;
    ptr.limit=sizeof(*gdt)-1;
    #if (MEMORY_DEBUG)
        print_to_serial("TSS address: ");
        print_hex_to_serial(mapped_tss_addr);
        print_to_serial("\n\r");
        print_to_serial("GDT address: ");
        print_hex_to_serial(mapped_gdt_addr);
        print_to_serial("\n\r");
        print_to_serial("&ptr: ");
        print_hex_to_serial((uint64)&ptr);
        print_to_serial("\n\r");
    #endif
    load_gdt(&ptr);

    gdt=mapped_gdt_addr;
    tss=mapped_tss_addr;

    print_to_serial("Mapped gdt address: ");
    print_hex_to_serial((uint64)gdt);
    print_to_serial("\n\rMapped tss address: ");
    print_hex_to_serial((uint64)tss);
    print_to_serial("\n\r");
}

void* map_page_to_kernel(void* address){
    uint64 pdpt=MEMORY_IO_PDPT;
    uint64 pd=(io_next_page>>18) & 511;
    uint64 pt=(io_next_page>>9) & 511;
    uint64 p = io_next_page & 511;
    map_page(pdpt,pd,pt,p,(uint64)address>>12,kernel_pml4,KERNEL_PAGEMAP_FLAGS);
    void* return_addr = (void*)((io_next_page<<12) | (pdpt << 39));
    io_next_page++;
    return return_addr;
}

void* malloc(uint64 size){
    #if(MEMORY_DEBUG)
        print_to_serial("Malloc issued\n\r");
    #endif
    heap_header* pos=heap;
    while((uint64)END_HEAP_HEADER(pos) < (uint64)heap + KERNEL_HEAP_SIZE_BYTES){
        #if(MEMORY_DEBUG)
            print_to_serial("Malloc looking at: ");
            print_hex_to_serial((uint64)pos);
            print_to_serial("\n\r");
        #endif
        if(pos->allocated==0){
            if(pos->length > size + 2*sizeof(heap_header)){ // Belefér az új header
                uint64 oldlength=pos->length;
                pos->allocated=1;
                pos->length=size;
                heap_header* end_header=END_HEAP_HEADER(pos);
                end_header->length=size;
                end_header->allocated=1;
                heap_header* next_header=NEXT_HEAP_HEADER(pos);
                next_header->allocated=0;
                next_header->length=oldlength-size-2*sizeof(heap_header);
                return (void*)((uint64)pos + sizeof(heap_header));
            }else if(pos->length >= size){ // Elég hosszú, de nem fér bele új header
                pos->allocated=1;
                END_HEAP_HEADER(pos)->allocated=0;
                return (void*)((uint64)pos + sizeof(heap_header));
            }
        }
        pos = NEXT_HEAP_HEADER(pos);//(heap_header*)((uint64)pos + pos->length + 2*sizeof(heap_header));
    }
    return NULL;
}

void free(void* address){
    #if(MEMORY_DEBUG)
        print_to_serial("Free issued\n\r");
    #endif
    heap_header* header = (heap_header*)((uint64)address - sizeof(heap_header));
    header->allocated=0;
    heap_header* end_header = END_HEAP_HEADER(header);
    end_header->allocated=0;
    if((uint64)header > (uint64)heap){
        heap_header* before_end_header=(heap_header*)((uint64)header-sizeof(heap_header));
        if(before_end_header->allocated==0){ // Össze kell olvastani az előzővel
            #if(MEMORY_DEBUG)
                print_to_serial("Free: Merging with prev segment\n\r");
            #endif
            heap_header* before_start_header=(heap_header*)((uint64)before_end_header - before_end_header->length - sizeof(heap_header));
            before_start_header->length += header->length + 2*sizeof(heap_header);
            header=before_start_header;
            end_header->length=header->length;
        }
    }
    if((uint64)NEXT_HEAP_HEADER(header) < (uint64)heap + KERNEL_HEAP_SIZE_BYTES){
        heap_header* next_header = NEXT_HEAP_HEADER(header);
        if(next_header->allocated==0){ // Össze kell olvasztani a következővel
            #if(MEMORY_DEBUG)
                print_to_serial("Free: Merging with next segment\n\r");
            #endif
            header->length += next_header->length + 2*sizeof(heap_header);
            end_header=END_HEAP_HEADER(next_header);
            end_header->length=header->length;
        }
    }
}

void map_page(uint64 pdpt, uint64 pd, uint64 pt, uint64 page, uint64 phys_page, uint64**** pml4, uint64 flags){
    uint64* pdpt_pointer=(uint64*)((uint64)(pml4[pdpt]) & PAGE_ADDR_MASK);
    if(pdpt_pointer==NULL){
        pdpt_pointer=malloc_page(1);
        pml4[pdpt]=(uint64***)((uint64)pdpt_pointer | flags);
        #if(MEMORY_DEBUG)
            print_to_serial("Allocated a page for a pdpt at :");
            print_hex_to_serial((uint64)pdpt_pointer);
            print_to_serial("\n\r");
        #endif
    }
    uint64* pd_pointer=(uint64*)((uint64)(pdpt_pointer[pd]) & PAGE_ADDR_MASK);
    if(pd_pointer==NULL){
        pd_pointer=malloc_page(1);
        pdpt_pointer[pd]=(uint64)pd_pointer | flags;
        #if(MEMORY_DEBUG)
            print_to_serial("Allocated a page for a pd at :");
            print_hex_to_serial((uint64)pd_pointer);
            print_to_serial("\n\r");
        #endif
    }
    uint64* pt_pointer=(uint64*)((uint64)(pd_pointer[pt]) & PAGE_ADDR_MASK);
    if(pt_pointer==NULL){
        pt_pointer=malloc_page(1);
        pd_pointer[pt]=(uint64)pt_pointer | flags;
        #if(MEMORY_DEBUG)
            print_to_serial("Allocated a page for a pt at :");
            print_hex_to_serial((uint64)pt_pointer);
            print_to_serial("\n\r");
        #endif
    }
    pt_pointer[page]=(phys_page << 12) | flags;
}

void inc_pmap_vars(uint64* pdpt, uint64* pd, uint64* pt, uint64* page, uint64**** pml4){
    (*page)++;
    if(*page==512){
        (*page)=0;
        (*pt)++;
        if(*pt==512){
            (*pt)=0;
            (*pd)++;
            if(*pd==512){
                (*pd)=0;
                (*pdpt)++;
            }
        }
    }
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