#include <efi.h>
#include <efilib.h>
#include <grp.h>
#include <types.h>
#include "gdt.h" //From partly from https://blog.llandsmeer.com/tech/2019/07/21/uefi-x64-userland.html
#include "../config/config.h"

// Functions
void crit_error(CHAR16* message);
EFI_FILE_HANDLE GetVolume(EFI_HANDLE image);
EFI_STATUS load_file(EFI_FILE_HANDLE handle, CHAR16* file_name, void** pool, uint64* file_size);
void* alloc_page(uint64 n);
uint64 FileSize(EFI_FILE_HANDLE FileHandle);
uint64 sqrt(uint64 n);
void inc_pmap_vars(uint64* pdpt, uint64* pd, uint64* pt, uint64* page, uint64**** pml4, uint64 flags);
void map_page(uint64 pdpt, uint64 pd, uint64 pt, uint64 page, uint64 phys_page, uint64**** pml4, uint64 flags);
extern void load_gdt(void* gdp_ptr);
extern void load_pml4(void* pml4);
extern void jmp_to_kernel(void* kernel_stack_vma, void* kernel_virt_addr, void* kernel_info_vma);
void outb(uint16 port, uint8 data);
void int_to_text_hex(uint64 n, char *string);
void int_to_text(uint64 n, uint8 string[]);
void print_hex_to_serial(uint64 n);
void print_to_serial(char *buf);
void print_int_to_serial(uint64 n);

// Structs
typedef volatile struct rgb{
    uint8 r;
    uint8 g;
    uint8 b;
} rgb;
typedef struct{
    uint64 real_address;
    uint64 frame_buffer;
    uint64 screen_width;
    uint64 screen_height;
    uint64 kernel_pml4_addresss;
    EFI_MEMORY_DESCRIPTOR* memmap;
    uint64 memmap_desc_count;
    uint64 memmap_desc_size;
    uint64 largest_area_idx;
    uint64 used_pages;
    void* gdt;
    void* tss;
    uint64 kernel_next_page;
    uint64 acpi_rsdp;
    uint64 io_space_used_pages;
} kernel_info;


// Global vars
void* largest_memory_part;
uint64 largest_memory_part_size; // pages
uint64 largest_memory_part_idx;
uint64 used_pages = 0;
uint64 sqrt_iter_count=1;
uint64**** pml4 = NULL;
EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
char hexletters[]="0123456789ABCDEF";
uint64 io_space_used_pages = 0;

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    uefi_call_wrapper(SystemTable->BootServices->SetWatchdogTimer,4,0,0,0,NULL);

    EFI_STATUS status;

    // Getting root directory handle
    EFI_FILE_HANDLE volume = GetVolume(ImageHandle);
    // Loading necessary files
    void* rawlogo;
    uint64 rawlogo_file_size;
    load_file(volume,L"rawlogo.bin",&rawlogo,&rawlogo_file_size);

    void* kernel_binary;
    uint64 kernel_binary_size;
    load_file(volume,L"kernel.bin",&kernel_binary,&kernel_binary_size);

    // Initializing screen
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    status=uefi_call_wrapper(BS->LocateProtocol,3,&gop_guid,NULL,(void**)&gop);
    if(EFI_ERROR(status)){
        crit_error(L"GOP not found.");
    }

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* gop_info;
    uint64 gop_info_size = 0;
    uint64 gop_mode_count = 0;
    uint64 native_mode = 0;
    uint64 width = 0;
    uint64 height = 0;
    uint32* framebuffer = NULL;
    uint64 framebuffer_size = 0;

    status=uefi_call_wrapper(gop->QueryMode,4,gop,(gop->Mode==NULL ? 0 : gop->Mode->Mode), &gop_info_size, &gop_info);
    if(status==EFI_NOT_STARTED){
        status=uefi_call_wrapper(gop->SetMode,2,gop,0);
    }
    if(EFI_ERROR(status)){
        crit_error(L"Unable to initialize screen.");
    }
    gop_mode_count=gop->Mode->MaxMode;
    native_mode=gop->Mode->Mode;

    status=uefi_call_wrapper(gop->QueryMode,4,gop,native_mode,&gop_info_size,&gop_info);
    if(EFI_ERROR(status)){
        crit_error(L"Unable to extract screen info");
    }
    framebuffer=(uint32*)(gop->Mode->FrameBufferBase);
    framebuffer_size=gop->Mode->FrameBufferSize;
    width = gop_info->HorizontalResolution;
    height = gop_info->VerticalResolution;
    #if(EFI_DEBUG)
        Print(L"Resolution: %dx%d; Pixel format: %d, Framebuffer: 0x%x\n\r",gop_info->HorizontalResolution,gop_info->VerticalResolution,gop_info->PixelFormat,framebuffer);
        Print(L"GDT address: 0x%x\n\r", &gdt_table);
    #endif

    // Getting the acpi rsdp
    EFI_GUID acpi_guid = ACPI_20_TABLE_GUID;//{0x8868e871,0xe4f1,0x11d3,0xbc,0x22,0x80,0xc7,0x3c,0x88,0x81};

    #if(EFI_DEBUG)
        Print(L"Number of UEFI system table entries: %d\n\r", SystemTable->NumberOfTableEntries);
        Print(L"System table entries: \n\r");
    #endif
    uint64 acpi_rsdp_addr;
    for(uint32 i=0;i<SystemTable->NumberOfTableEntries;i++){
        #if(EFI_DEBUG)
            Print(L"#%d GUID: %x-%x-%x-%x-00",
                i,
                SystemTable->ConfigurationTable[i].VendorGuid.Data1,
                SystemTable->ConfigurationTable[i].VendorGuid.Data2,
                SystemTable->ConfigurationTable[i].VendorGuid.Data3,
                SystemTable->ConfigurationTable[i].VendorGuid.Data4[0]<<8 | SystemTable->ConfigurationTable[i].VendorGuid.Data4[1]                    
            );
            for (uint8 j=0;j<6;j++){
                Print(L"%x",
                    SystemTable->ConfigurationTable[i].VendorGuid.Data4[2+j]
                );
            }
            Print(L"; Pointer: 0x%x;",
                SystemTable->ConfigurationTable[i].VendorTable
            );
        #endif
        if(CompareGuid(&(SystemTable->ConfigurationTable[i].VendorGuid),&acpi_guid)==0){
            #if(EFI_DEBUG)
                Print(L" ACPI root table;");
            #endif
            acpi_rsdp_addr=(uint64)(SystemTable->ConfigurationTable[i].VendorTable);
        }
        #if(EFI_DEBUG)
            Print(L"\n\r");
        #endif
    }

    // Getting the memory map
    uint64 mem_map_size = 100;
    uint64 mem_map_size_out=mem_map_size;
    EFI_MEMORY_DESCRIPTOR* buffer;
    uint64 mem_map_desc_size;
    uint64 mem_map_key;
    uint32 mem_map_version;
    uint64 mem_map_desc_count;
    uint64 ram_amount;

    do{
        buffer=AllocatePool(mem_map_size);
        if(buffer==NULL)break;
        status=uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap,5,
            &mem_map_size_out,
            buffer,
            &mem_map_key,
            &mem_map_desc_size,
            &mem_map_version
        );
        if(EFI_ERROR(status)){
            FreePool(buffer);
            mem_map_size+=100;
        }
    }while(status!=EFI_SUCCESS);
    
    if(buffer==NULL){
        crit_error(L"Failed to retrieve the memory map");
    }

    mem_map_desc_count=mem_map_size_out / mem_map_desc_size;
    #if(EFI_DEBUG)
        Print(L"Memory map size: %u, Memory map descriptor size: %u, Desc count: %u\n\r",mem_map_size_out,mem_map_desc_size,mem_map_desc_count);
    #endif
    largest_memory_part_size=0;
    EFI_MEMORY_DESCRIPTOR* mem_desc = buffer;
    for(uint32 i=0;i<mem_map_desc_count;i++){
        #if(EFI_DEBUG)
        Print(L"Type: %d PhsyicalStart: %lx VirtualStart: %lx NumberofPages: %d Attribute %lx\n",
            mem_desc->Type, mem_desc->PhysicalStart,
            mem_desc->VirtualStart, mem_desc->NumberOfPages,
            mem_desc->Attribute);
        #endif
        if(mem_desc->NumberOfPages > largest_memory_part_size && mem_desc->Type==EfiConventionalMemory){
            largest_memory_part_size = mem_desc->NumberOfPages;
            largest_memory_part = (void*) (mem_desc->PhysicalStart);
            largest_memory_part_idx=i;
        }
        ram_amount=mem_desc->PhysicalStart+((mem_desc->NumberOfPages)<<12);
        mem_desc= (EFI_MEMORY_DESCRIPTOR*)((uint64)mem_desc + mem_map_desc_size);
    }

    EFI_MEMORY_DESCRIPTOR* memmap = alloc_page((mem_map_size_out >> 12) + 1);
    for(uint32 i=0;i<mem_map_size_out;i++)((uint8*)memmap)[i]=((uint8*)buffer)[i];
    FreePool(buffer);

    #if(EFI_DEBUG)
        Print(L"Ram amount: %u MB\n\r",ram_amount >> 20);
    #endif

    // Draw the logo
    uint32 logo_width = ((uint32*)rawlogo)[0];
    uint32 logo_height = ((uint32*)rawlogo)[1];

    uint32 startx=width/2-logo_width/2;
    uint32 starty=height/2-logo_height/2;
    uint32 centerx=width/2;
    uint32 centery=height/2;
    uint32 distance=0;
    uint32 maxdistance=sqrt(centerx*centerx+centery*centery);
    const uint8 gradient_start=0x24;

    rgb *pixel=(rgb*)(rawlogo+8);

    for(uint32 y=0;y<height;y++)for(uint32 x=0;x<width;x++){
        distance=sqrt((x-centerx)*(x-centerx) + (y-centery)*(y-centery));
        uint8 tmp=gradient_start-(uint8)((distance*gradient_start)/maxdistance);
        framebuffer[y*width + x]=tmp | (tmp << 8) | (tmp << 16);
    }
    for(uint32 y=0;y<logo_height;y++){
        for(uint32 x=0;x<logo_width;x++){
            framebuffer[(starty + y)*width + startx + x]= ((uint32)pixel->r << 16) | (((uint32)pixel->g) << 8) | (((uint32)pixel->b)); //(*(uint32*)pixel) & 0x00ffffff;
            pixel++;
        }
    }

    // Creating the page table
    // Identity mapping the memory
    uint64 pdpt_index=0;
    uint64 pd_index=0;
    uint64 pt_index=0;
    uint64 page_index=0;
    uint64 flags=0b011; // bit0: present, bit1: read/write, bit2: clear: supervisor only
    uint64 page_count = ram_amount >> 12;

    pml4=alloc_page(1);
    #if(EFI_DEBUG)
        Print(L"PML4 address: 0x%x\n\r",(uint64)pml4);
    #endif

    for(uint64 i=0;i<page_count;i++){
        map_page(pdpt_index,pd_index,pt_index,page_index,i,pml4,flags);
        inc_pmap_vars(&pdpt_index,&pd_index,&pt_index,&page_index,pml4,flags);
    }

    // Creating the kernel address space
    pdpt_index=KERNEL_VMA_PDPT;
    pd_index=KERNEL_BINARY_PD;
    pt_index=0;
    page_index=0;
    flags=0b011; // bit0: present, bit1: read/write, bit2: clear: supervisor only

    // Copying the kernel binary to an allocated space at a page boundary
    uint32 kernel_binary_size_page = (kernel_binary_size >> 12) + 1;
    void* kernel=alloc_page(kernel_binary_size_page);
    for(uint32 i=0;i<kernel_binary_size;i++){
        ((uint8*)kernel)[i]=((uint8*)kernel_binary)[i];
    }

    uint64 kernel_start_pageframe=((uint64)kernel) >> 12;
    
    for(uint32 i=0;i<kernel_binary_size_page;i++){
        map_page(pdpt_index,pd_index,pt_index,page_index,kernel_start_pageframe + i,pml4,flags);
        inc_pmap_vars(&pdpt_index,&pd_index,&pt_index,&page_index,pml4,flags);
    }

    // Kernel info
    pd_index=KERNEL_INFO_START_PD;
    pt_index=0;
    page_index=0;

    uint32 kernel_info_size_page=(sizeof(kernel_info) >> 12) + 1;
    kernel_info* kernel_info_pointer = alloc_page(kernel_info_size_page);

    for(uint32 i=0;i<kernel_info_size_page;i++){
        map_page(pdpt_index,pd_index,pt_index,page_index,((uint64)kernel_info_pointer >> 12) + i,pml4,flags);
        inc_pmap_vars(&pdpt_index,&pd_index,&pt_index,&page_index,pml4,flags);
    }

    // Secondary kernel stack
    pd_index=KERNEL_SECONDARY_STACK_START_PD;
    pt_index=0;
    page_index=0;
    
    void* kernel_secondary_stack = alloc_page(KERNEL_SECONDARY_STACK_SIZE);
    uint64 kernel_secondary_stack_start_pageframe = (uint64)kernel_secondary_stack >> 12;
    for(uint32 i=0;i<KERNEL_SECONDARY_STACK_SIZE;i++){
        map_page(pdpt_index,pd_index,pt_index,page_index,kernel_secondary_stack_start_pageframe + i,pml4,flags);
        inc_pmap_vars(&pdpt_index,&pd_index,&pt_index,&page_index,pml4,flags);
    }

    // Kernel stack
    pd_index=KERNEL_STACK_START_PD;
    pt_index=0;
    page_index=0;
    
    void* kernel_stack = alloc_page(KERNEL_STACK_SIZE);
    uint64 kernel_stack_start_pageframe = (uint64)kernel_stack >> 12;
    for(uint32 i=0;i<KERNEL_STACK_SIZE;i++){
        map_page(pdpt_index,pd_index,pt_index,page_index,kernel_stack_start_pageframe + i,pml4,flags);
        inc_pmap_vars(&pdpt_index,&pd_index,&pt_index,&page_index,pml4,flags);
    }

    // Kernel heap
    pd_index=KERNEL_HEAP_START_PD;
    pt_index=0;
    page_index=0;

    void* kernel_heap=alloc_page(KERNEL_HEAP_SIZE);
    uint64 kernel_heap_start_pageframe=(uint64)kernel_heap >> 12;
    for(uint32 i=0;i<KERNEL_HEAP_SIZE;i++){
        map_page(pdpt_index,pd_index,pt_index,page_index,kernel_heap_start_pageframe + i,pml4,flags);
        inc_pmap_vars(&pdpt_index,&pd_index,&pt_index,&page_index,pml4,flags);
    }

    //Framebuffer

    pdpt_index=MEMORY_IO_PDPT;
    pd_index=(io_space_used_pages>>18) & 511;
    pt_index=(io_space_used_pages>>9) & 511;
    page_index=io_space_used_pages & 511;
    void* framebuffer_vma = (void*)((MEMORY_IO_PDPT << 39) + io_space_used_pages);
    uint64 framebuffer_size_page=framebuffer_size >> 12;
    uint64 framebuffer_start_pageframe=(uint64)framebuffer >> 12;
    io_space_used_pages+=framebuffer_size_page;
    for(uint64 i=0;i<framebuffer_size_page;i++){
        map_page(pdpt_index,pd_index,pt_index,page_index,framebuffer_start_pageframe + i,pml4,flags);
        inc_pmap_vars(&pdpt_index,&pd_index,&pt_index,&page_index,pml4,flags);
    }

    // Exiting boot services
    uefi_call_wrapper(BS->ExitBootServices,2,ImageHandle,mem_map_key);
    asm("cli");

    // Creating and loading the gdt
    for(uint32 i=0;i<sizeof(tss);i++){
        ((uint8*)&tss)[i]=0;
    }
    uint64 tss_base=(uint64)&tss;
    gdt_table.tss_low.base15_0 = tss_base & 0xffff;
    gdt_table.tss_low.base23_16 = (tss_base >> 16) & 0xff;
    gdt_table.tss_low.base31_24 = (tss_base >> 24) & 0xff;
    gdt_table.tss_low.limit15_0 = sizeof(tss);
    gdt_table.tss_high.limit15_0 = (tss_base >> 32) & 0xffff;
    gdt_table.tss_high.base15_0 = (tss_base >> 48) & 0xffff;
    tss.iopb_offset=sizeof(tss);

    struct table_ptr gdt_ptr = { sizeof(gdt_table)-1, (uint64)&gdt_table };
    load_gdt(&gdt_ptr);

    // Loading the pml4
    load_pml4(pml4);


    // Filling out the kernel_info data
    kernel_info* kernel_info_vma=(kernel_info*)((KERNEL_VMA_PDPT << 39) | (KERNEL_INFO_START_PD << 30));
    void* kernel_stack_vma=(void*)((KERNEL_VMA_PDPT << 39) | (KERNEL_STACK_START_PD << 30));
    void* kernel_stack_top_vma=(void*)((KERNEL_VMA_PDPT << 39) | (KERNEL_STACK_START_PD << 30)) + (KERNEL_STACK_SIZE << 12)-8;
    void* kernel_vma=(void*)((KERNEL_VMA_PDPT << 39) | (KERNEL_BINARY_PD << 30));

    kernel_info_vma->frame_buffer=(uint64)framebuffer_vma;
    kernel_info_vma->kernel_pml4_addresss=(uint64)pml4;
    kernel_info_vma->screen_width=(uint64)width;
    kernel_info_vma->screen_height=(uint64)height;
    kernel_info_vma->real_address=(uint64)kernel;
    kernel_info_vma->largest_area_idx=largest_memory_part_idx;
    kernel_info_vma->memmap_desc_size=mem_map_desc_size;
    kernel_info_vma->memmap_desc_count=mem_map_desc_count;
    kernel_info_vma->memmap=memmap;
    kernel_info_vma->used_pages=used_pages;
    kernel_info_vma->gdt=&gdt_table;
    kernel_info_vma->tss=&tss;
    kernel_info_vma->acpi_rsdp=acpi_rsdp_addr;
    kernel_info_vma->io_space_used_pages=io_space_used_pages;

    #if(EFI_DEBUG)
        print_to_serial("Kernel_vma: ");
        print_hex_to_serial((uint64)kernel_vma);
        print_to_serial("\n\rKernel_stack_vma: ");
        print_hex_to_serial((uint64)kernel_stack_vma);
        print_to_serial("\n\r");
    #endif

    jmp_to_kernel(kernel_stack_top_vma,kernel_vma,kernel_info_vma);

    while(1);
}

void crit_error(CHAR16* message){
    Print(message);
    while(1);
}

EFI_FILE_HANDLE GetVolume(EFI_HANDLE image) //From osdev
{
    EFI_LOADED_IMAGE *loaded_image = NULL;                  /* image interface */
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;      /* image interface GUID */
    EFI_FILE_IO_INTERFACE *IOVolume;                        /* file system interface */
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID; /* file system interface GUID */
    EFI_FILE_HANDLE Volume;                                 /* the volume's interface */

    /* get the loaded image protocol interface for our "image" */
    uefi_call_wrapper(BS->HandleProtocol, 3, image, &lipGuid, (void **) &loaded_image);
    /* get the volume handle */
    uefi_call_wrapper(BS->HandleProtocol, 3, loaded_image->DeviceHandle, &fsGuid, (VOID*)&IOVolume);
    uefi_call_wrapper(IOVolume->OpenVolume, 2, IOVolume, &Volume);
    return Volume;
}

EFI_STATUS load_file(EFI_FILE_HANDLE handle, CHAR16* file_name, void** pool, uint64* file_size){
    EFI_FILE_HANDLE file;
    EFI_STATUS status = uefi_call_wrapper(handle->Open,5,handle,&file,file_name,EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
    *file_size = FileSize(file);
    *pool = AllocatePool(*file_size);
    uefi_call_wrapper(file->Read, 3, file, file_size, *pool);
    uefi_call_wrapper(file->Close, 1, file);
}

void* alloc_page(uint64 n){
    if(used_pages+n<largest_memory_part_size){
        void* return_addr=(void*)((uint64)largest_memory_part+used_pages*4096);
        for(uint64 i=(uint64)return_addr;i<(uint64)return_addr+n*4096;i++){*(uint8*)i=0;}
        used_pages+=n;
        return return_addr;
    }else{
        crit_error(L"Out of memory");
    }
}

uint64 FileSize(EFI_FILE_HANDLE FileHandle)
{
  uint64 ret;
  EFI_FILE_INFO       *FileInfo;         /* file information structure */
  /* get the file's size */
  FileInfo = LibFileInfo(FileHandle);
  ret = FileInfo->FileSize;
  FreePool(FileInfo);
  return ret;
}

uint64 sqrt(uint64 n){
  uint64 lo = 0, hi = n, mid;
  for(uint64 i = 0 ; i < sqrt_iter_count ; i++){
      mid = (lo+hi)>>1;
      if(mid*mid == n) return mid;
      if(mid*mid > n) hi = mid;
      else lo = mid;
  }
  return mid;
}

void map_page(uint64 pdpt, uint64 pd, uint64 pt, uint64 page, uint64 phys_page, uint64**** pml4, uint64 flags){
    //((uint64*)(((uint64)((uint64*)((uint64)(((uint64*)((uint64)pml4[pdpt] & PAGE_ADDR_MASK))[pd])& PAGE_ADDR_MASK)) [pt]) & PAGE_ADDR_MASK))[virt_page_in_pt] = (phys_page << 12) | flags;
    uint64* pdpt_pointer=(uint64*)((uint64)(pml4[pdpt]) & PAGE_ADDR_MASK);
    if(pdpt_pointer==NULL){
        pdpt_pointer=alloc_page(1);
        pml4[pdpt]=(uint64***)((uint64)pdpt_pointer | flags);
    }
    uint64* pd_pointer=(uint64*)((uint64)(pdpt_pointer[pd]) & PAGE_ADDR_MASK);
    if(pd_pointer==NULL){
        pd_pointer=alloc_page(1);
        pdpt_pointer[pd]=(uint64)pd_pointer | flags;
    }
    uint64* pt_pointer=(uint64*)((uint64)(pd_pointer[pt]) & PAGE_ADDR_MASK);
    if(pt_pointer==NULL){
        pt_pointer=alloc_page(1);
        pd_pointer[pt]=(uint64)pt_pointer | flags;
    }
    pt_pointer[page]=(phys_page << 12) | flags;
}

void inc_pmap_vars(uint64* pdpt, uint64* pd, uint64* pt, uint64* page, uint64**** pml4, uint64 flags){
    (*page)++;
    if(*page==512){
        #if(EFI_DEBUG)
            Print(L"Pagemap: pdpt: %u, pd: %u, pt: %u, page: %u\n\r",
                *pdpt,
                *pd,
                *pt,
                *page
            );
        #endif
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
    /*if(*page==512){
        (*page)=0;
        (*pt)++;
        if(*pt!=512){
            ((uint64*)((uint64)(((uint64*)((uint64)pml4[*pdpt] & PAGE_ADDR_MASK))[*pd])& PAGE_ADDR_MASK)) [*pt]=(uint64)alloc_page(1) | flags;
        }else{
            (*pd)++;
            (*pt)=0;
            if(*pd!=512){
                ((uint64*)((uint64)pml4[*pdpt] & PAGE_ADDR_MASK))[*pd]=(uint64)((uint64)alloc_page(1) | flags);
                ((uint64*)((uint64)(((uint64*)((uint64)pml4[*pdpt] & PAGE_ADDR_MASK))[*pd])& PAGE_ADDR_MASK)) [*pt]=(uint64)((uint64)alloc_page(1) | flags);
            }
            else {
                (*pdpt)++;
                (*pd)=0;
                pml4[*pdpt]=(uint64***)((uint64)alloc_page(1) | flags);
                ((uint64*)((uint64)pml4[*pdpt] & PAGE_ADDR_MASK))[*pd]=(uint64)alloc_page(1) | flags;
                ((uint64*)((uint64)(((uint64*)((uint64)pml4[*pdpt] & PAGE_ADDR_MASK))[*pd])& PAGE_ADDR_MASK)) [*pt]=(uint64)((uint64)alloc_page(1) | flags);
            }
        }
    }*/
}

void outb(uint16 port, uint8 data)
{
    asm volatile("outb %0, %1" : : "a" (data), "Nd" (port));
    return;
}

void int_to_text_hex(uint64 n, char *string){
    uint64 div=n;
    uint8 index=0;
    if(div==0){
        string[0]='0';
        return;
    }
    char buffer[21];
    while (div>0){
        buffer[index]=*(hexletters+div%16);
        div=(uint64)div/16;
        index++;
    }
    for(uint8 i=0;i<21;i++){string[i]=0;}
    for(uint8 i=0;i<index;i++){
        string[i]=buffer[index-i-1];
    }
    return;
}
void int_to_text(uint64 n, uint8 string[]){
    uint64 div=n;
    uint8 index=0;
    if(div==0){
        string[0]='0';
        return;
    }
    char buffer[21];
    while (div>0){
        buffer[index]=div%10+'0';
        div=(uint64)div/10;
        index++;
    }
    for(uint8 i=0;i<21;i++){string[i]=0;}
    for(uint8 i=0;i<index;i++){
        string[i]=buffer[index-i-1];
    }
    return;
}

void print_hex_to_serial(uint64 n){
    char buf[30];
    for(uint8 i=0;i<30;i++)buf[i]=0;
    buf[0]='0';
    buf[1]='x';
    int_to_text_hex(n,buf+2);
    print_to_serial(buf);
}
void print_to_serial(char *buf){
    uint64 i=0;
    while(buf[i]!=0){
        outb(SERIAL_PORT,buf[i]);
        i++;
    }
}
void print_int_to_serial(uint64 n){
    char buf[30];
    for(uint8 i=0;i<30;i++)buf[i]=0;
    int_to_text(n,buf);
    print_to_serial(buf);
}
