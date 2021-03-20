#include <efi.h>
#include <efilib.h>
#include <grp.h>
#include <types.h>
#include "gdt.h" //From https://blog.llandsmeer.com/tech/2019/07/21/uefi-x64-userland.html
#define KEYBOARD_PORT 0x60
#define SERIAL_PORT 0x3F8 

/* bitflags */
#define PAGE_BIT_P_PRESENT (1<<0)
#define PAGE_BIT_RW_WRITABLE (1<<1)
#define PAGE_BIT_US_USER (1<<2)
#define PAGE_XD_NX (1<<63)

/* bit mask for page aligned 52-bit address */
#define PAGE_ADDR_MASK 0x000ffffffffff000

/* these get updated when a page is accessed/written to */
#define PAGE_BIT_A_ACCESSED (1<<5)
#define PAGE_BIT_D_DIRTY (1<<6)

#define KERNEL_START_PDPT 4
#define KERNEL_START_PD 0
#define KERNEL_START_PT 0
/*#define KERNEL_START_P 0*/

extern void load_gdt(struct table_ptr * gdt_ptr);
extern void load_pml4(void* pml4);
extern void jmp_to_kernel(void* stack, void* address, void* kernel_info);

UINT64 FileSize(EFI_FILE_HANDLE FileHandle)
{
  UINT64 ret;
  EFI_FILE_INFO       *FileInfo;         /* file information structure */
  /* get the file's size */
  FileInfo = LibFileInfo(FileHandle);
  ret = FileInfo->FileSize;
  FreePool(FileInfo);
  return ret;
}
uint8 inb(uint16 port){
    uint8 ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
    return ret;
}
uint16 inb16(uint16 port){
    uint16 ret;
    __asm__ volatile("inw %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}
uint32 inb32(uint16 port){
    uint32 ret = 0;
    asm volatile("inl %1, %0" : "=a"(ret) : "d"(port));
    return ret;
}
void outb(uint16 port, uint8 data)
{
  asm volatile("outb %0, %1" : : "a" (data), "Nd" (port));
  return;
}
void outb16(uint16 port, uint16 data)
{
  asm volatile("outw %0, %1" : : "a" (data), "Nd" (port));
  return;
}
void outb32(uint16 port, uint32 data){
    asm volatile("outl %0, %1" : : "a"(data), "Nd" (port));
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
void getMemoryMap();
volatile typedef struct{
    uint16 length;
    uint64 address;
} GDTPointer;

volatile uint8 GDT[]={
    //Null
    0xFF,0xFF,  //limit
    0, 0,       //base (Low)
    0,          //base (Middle)
    0,          //Access
    1,          //Granuality
    0,          //Base (High)
    //Code
    0, 0,       //Limit
    0, 0,       //Base (Low)
    0,          //Base (Middle)
    0b10011010, //Access
    0b10101111, //Granuality, 64bits flag, limit 19:16
    0,          //Base (High)
    //Data
    0, 0,       //Limit
    0, 0,       //Base (Low)
    0,          //Base (Middle)
    0b10010010, //Access
    0b00000000, //Granuality, 64bits flag, limit 19:16
    0,          //Base (High)
};

typedef struct{
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
    
} kernel_info;

GDTPointer GDTDesc;

__attribute__((aligned(4096)))
uint64*** kernel_pml4[512];

uint32* framebuffer;
uint32 width=0;
uint32 height=0;

uint64 best_memory_part_base=0;
uint64 best_memory_part_length=0;
uint64 page_idx=0;
uint64 ram_amount=0;

/*__attribute__((aligned(4096)))
uint64 pml4[512];*/

void* alloc_page(uint64 n){
    if(page_idx+n<best_memory_part_length){
        void* return_addr=(void*)(best_memory_part_base+page_idx*4096);
        for(uint64 i=(uint64)return_addr;i<(uint64)return_addr+n*4096;i++){*(uint8*)i=0;}
        page_idx+=n;
        return return_addr;
    }else return (void*)0;
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) //Mostly from osdev
{
    InitializeLib(ImageHandle, SystemTable);
    SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_STATUS s;
 
    s = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol,
                               3,
                              ImageHandle,
                              &LoadedImageProtocol,
                              (void **)&loaded_image);
    if (EFI_ERROR(s)) {
        Print(L"handleprotocol: %r\n", s);
    }
 
    Print(L"Image base: 0x%lx\n", loaded_image->ImageBase);

    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    if(EFI_ERROR(status))Print(L"GOP not found");
    else{
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
        UINTN SizeOfInfo, numModes, nativeMode;
        
        status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);
        // this is needed to get the current video mode
        if (status == EFI_NOT_STARTED)
            status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
        if(EFI_ERROR(status)) {
            Print(L"Unable to get native mode");
        } else {
            nativeMode = gop->Mode->Mode;
            numModes = gop->Mode->MaxMode;
            status = uefi_call_wrapper(gop->QueryMode, 4, gop, nativeMode, &SizeOfInfo, &info);
            Print(L"Native mode: %03d width %d height %d format %x%s\n",
                nativeMode,
                info->HorizontalResolution,
                info->VerticalResolution,
                info->PixelFormat
            );
            Print(L"Framebuffer address %x size %d, width %d height %d pixelsperline %d \n",
                gop->Mode->FrameBufferBase,
                gop->Mode->FrameBufferSize,
                gop->Mode->Info->HorizontalResolution,
                gop->Mode->Info->VerticalResolution,
                gop->Mode->Info->PixelsPerScanLine
            );
            framebuffer=(uint32*)gop->Mode->FrameBufferBase;
            width=gop->Mode->Info->HorizontalResolution;
            height=gop->Mode->Info->VerticalResolution;

            EFI_FILE_HANDLE Volume = GetVolume(ImageHandle);
            //uint16 file_name[]=;
            CHAR16 *FileName=L"rawlogo.bin";
            EFI_FILE_HANDLE logofile;
            EFI_STATUS status = uefi_call_wrapper(Volume->Open, 5, Volume, &logofile, FileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
            if(status!=EFI_SUCCESS){
                Print(L"Error, couldnt open logo file");
                while(1);
            }
            uint64 logofilesize=FileSize(logofile);
            Print(L"Logo size: %d\n",logofilesize);
            uint8* rawlogo=AllocatePool(logofilesize);
            uefi_call_wrapper(logofile->Read, 3, logofile, &logofilesize, rawlogo);
            uefi_call_wrapper(logofile->Close, 1, logofile);

            FileName=L"kernel.bin";
            EFI_FILE_HANDLE kernelfile;
            status = uefi_call_wrapper(Volume->Open, 5, Volume, &kernelfile, FileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
            if(status!=EFI_SUCCESS){
                Print(L"Error, couldn't open kernel.bin");
                while(1);
            }
            uint64 kernelfilesize=FileSize(kernelfile);
            uint8* kernel_file=AllocatePool(kernelfilesize);
            uefi_call_wrapper(kernelfile->Read, 3, kernelfile, &kernelfilesize, kernel_file);
            uefi_call_wrapper(kernelfile->Close, 1, kernelfile);
            
            // Memory map, from some random website on the internet
            status = EFI_SUCCESS;
            UINTN MemMapSize = sizeof(EFI_MEMORY_DESCRIPTOR)*16;
            UINTN MemMapSizeOut = MemMapSize;
            UINTN MemMapKey = 0; UINTN MemMapDescriptorSize = 0;
            UINT32 MemMapDescriptorVersion = 0;
            UINTN DescriptorCount = 0;
            UINTN i = 0;
            uint8_t* buffer = NULL;
            EFI_MEMORY_DESCRIPTOR* MemoryDescriptorPtr = NULL;
            uint64 best_mem_part_idx;
            EFI_MEMORY_DESCRIPTOR* memmap;

            do 
            {
                buffer = AllocatePool(MemMapSize);
                if ( buffer == NULL ) break;

                status = uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap,5,&MemMapSizeOut, (EFI_MEMORY_DESCRIPTOR*)buffer, 
                    &MemMapKey, &MemMapDescriptorSize, &MemMapDescriptorVersion);

                Print(L"MemoryMap: Status %x\n", status);
                if ( status != EFI_SUCCESS )
                {
                    FreePool(buffer);
                    MemMapSize += sizeof(EFI_MEMORY_DESCRIPTOR)*16;
                }
            } while ( status != EFI_SUCCESS );

            if ( buffer != NULL )
            {
                DescriptorCount = MemMapSizeOut / MemMapDescriptorSize;
                MemoryDescriptorPtr = (EFI_MEMORY_DESCRIPTOR*)buffer;
                memmap=(EFI_MEMORY_DESCRIPTOR*)buffer;

                Print(L"MemoryMap: DescriptorCount %d\n", DescriptorCount);

                for ( i = 0; i < DescriptorCount; i++ )
                {
                    MemoryDescriptorPtr = (EFI_MEMORY_DESCRIPTOR*)(buffer + (i*MemMapDescriptorSize));
                    Print(L"Type: %d PhsyicalStart: %lx VirtualStart: %lx NumberofPages: %d Attribute %lx\n",
                        MemoryDescriptorPtr->Type, MemoryDescriptorPtr->PhysicalStart,
                        MemoryDescriptorPtr->VirtualStart, MemoryDescriptorPtr->NumberOfPages,
                        MemoryDescriptorPtr->Attribute);
                    if(MemoryDescriptorPtr->Type==EfiConventionalMemory && MemoryDescriptorPtr->NumberOfPages>best_memory_part_length){
                        best_memory_part_length=MemoryDescriptorPtr->NumberOfPages;
                        best_memory_part_base=MemoryDescriptorPtr->PhysicalStart;
                        best_mem_part_idx=i;
                        //Print(L"Longer; physical start: 0x%x\n")
                    }
                    ram_amount=MemoryDescriptorPtr->PhysicalStart+(MemoryDescriptorPtr->NumberOfPages)*4096;
                }
                //EFI_MEMORY_DESCRIPTOR* last_descriptor = ((EFI_MEMORY_DESCRIPTOR*)buffer+(DescriptorCount-1)*MemMapDescriptorSize);
                //FreePool(buffer);
            }else{
                Print(L"Error, buffer size is zero.");
                while(1);
            }

            memmap=alloc_page(((MemMapSizeOut/4096)+1)*4096);
            for(uint32 i=0;i<MemMapSizeOut;i++)((uint8*)memmap)[i]=buffer[i];

            Print(L"Ram amount: %u MiB\n", ram_amount/(1024*1024));

            Print(L"Longest memory start: 0x%lx, length: %d pages, amount: %d KiB\n", best_memory_part_base,best_memory_part_length, best_memory_part_length*4);
            Print(L"Memory amount: %u MiB\n",ram_amount/(1024*1024));
            Print(L"Kernel PML4 pointer: 0x%lx \n",kernel_pml4);
            Print(L"Tss (Bootloader): 0x%x\n",(uint64)&tss);

            uint32 logowidth = *(uint32*)rawlogo;
            uint32 logoheight = ((uint32*)rawlogo)[1];

            Print(L"Logowidth: %d, height: %d \n", logowidth, logoheight);

            uint32 startx=width/2-logowidth/2;
            uint32 starty=height/2-logoheight/2;

            rgb *pixel=(rgb*)(rawlogo+8);

            //Print(L"First pixel: R: %u, G: %u, B: %u \n",pixels[0].r,pixels[0].g,pixels[0].b);
            for(uint64 i=0;i<width*height;i++)framebuffer[i]=0x00181818;

            for(uint32 y=0;y<logoheight;y++){
                for(uint32 x=0;x<logowidth;x++){
                    framebuffer[starty*width + startx + y*width + x]=(uint32)(pixel->r)<<16 | (uint32)(pixel->g)<<8 | (uint32)(pixel->b);
                    pixel++;
                }
            }


            uefi_call_wrapper(BS->ExitBootServices,2,ImageHandle,MemMapKey);
            asm("cli");
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

            struct table_ptr gdt_ptr = { sizeof(gdt_table)-1, (uint64)&gdt_table };
            load_gdt(&gdt_ptr);

            //kernel_pml4=alloc_page(1);
            /*uint64 num_of_pages=ram_amount/4096;
            uint64 num_of_page_tables=num_of_pages/512+1;
            uint64 num_of_page_directories=num_of_page_tables/512+1;
            uint64 num_of_page_dir_pointer_tables=num_of_page_tables/512+1;*/

            uint64 pt_index=0;
            uint64 pd_index=0;
            uint64 pdpt_index=0;

            uint64 flags=0b111; //flags: Present bit0; Read\Write bit1; User bit2;
            uint64 no_page=ram_amount/4096;
            //if(ram_amount<(uint64)framebuffer+width*height*4)no_page=((uint64)framebuffer+width*height*4)/4096;
            kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
            /*kernel_pml4[pdpt_index][pd_index]=(uint64**)((uint64)alloc_page(1) | flags);
            kernel_pml4[pdpt_index][pd_index][pt_index]=(uint64*)((uint64)alloc_page(1) | flags);*/
            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)alloc_page(1) | flags;
            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);

            double lambdaw=(double)(logowidth+3)/(double)no_page*(double)4;
            double lambdah=(double)(logoheight+3)/(double)no_page*(double)4;
            uint64 quarter=no_page/4;

            for(uint64 i=0;i<no_page;i++){
                //if(page_idx+1==best_memory_part_length){for(uint64 i=0;i<width*height;i++) framebuffer[i]=0x00ff0000;break;}
                /*uint64 t=(uint64)(lambda*(double)i);
                framebuffer[t]=0x00ff0000;
                framebuffer[t+width]=0x00ff0000;
                framebuffer[t+width*2]=0x00ff0000;*/

                if(i<quarter){
                    framebuffer[(starty-3)*width+startx+(uint64)((double)i*lambdaw)]=0x00ff0000;
                    framebuffer[(starty-2)*width+startx+(uint64)((double)i*lambdaw)]=0x00ff0000;
                    framebuffer[(starty-1)*width+startx+(uint64)((double)i*lambdaw)]=0x00ff0000;
                }else if(i<quarter*2){
                    framebuffer[(starty+(uint64)((double)(i-quarter)*lambdah))*width+startx+logowidth+2]=0x00ff0000;
                    framebuffer[(starty+(uint64)((double)(i-quarter)*lambdah))*width+startx+logowidth+1]=0x00ff0000;
                    framebuffer[(starty+(uint64)((double)(i-quarter)*lambdah))*width+startx+logowidth]=0x00ff0000;
                }else if(i<quarter*3){
                    framebuffer[(starty+logoheight+2)*width+startx+(uint64)((double)(quarter-(i-quarter*2))*lambdaw)-2]=0x00ff0000;
                    framebuffer[(starty+logoheight+1)*width+startx+(uint64)((double)(quarter-(i-quarter*2))*lambdaw)-2]=0x00ff0000;
                    framebuffer[(starty+logoheight+0)*width+startx+(uint64)((double)(quarter-(i-quarter*2))*lambdaw)-2]=0x00ff0000;
                }else{
                    framebuffer[(starty+(uint64)((double)((quarter-(i-quarter*3))*lambdah))-3)*width+startx-2]=0x00ff0000;
                    framebuffer[(starty+(uint64)((double)((quarter-(i-quarter*3))*lambdah))-3)*width+startx-1]=0x00ff0000;
                    framebuffer[(starty+(uint64)((double)((quarter-(i-quarter*3))*lambdah))-3)*width+startx]=0x00ff0000;
                }

                //kernel_pml4[pdpt_index][pd_index][pt_index][i%512]=(i*4096) | flags;
                ((uint64*)(((uint64)((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]) & PAGE_ADDR_MASK))[i%512] = i*4096 | flags;
                if((i+1)%512==0){
                    pt_index++;
                    if(pt_index!=512){
                        ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)alloc_page(1) | flags;
                    }else{
                        pd_index++;
                        pt_index=0;
                        if(pd_index!=512){
                            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)((uint64)alloc_page(1) | flags);
                            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                        }
                        else {
                            pdpt_index++;
                            pd_index=0;
                            kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
                            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)alloc_page(1) | flags;
                            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                        }
                    }
                }
                //framebuffer[i]=0x00ff0000;
            }

            uint64 framebuffer_start_pageframe=(uint64)framebuffer/4096;
            uint64 framebuffer_size_page=(width*height*4)/4096+1;
            pdpt_index=(uint64)framebuffer>>39;
            pd_index=((uint64)framebuffer>>30) & 0x1ff;
            pt_index=((uint64)framebuffer>>21) & 0x1ff;
            //page_idx=((uint64)framebuffer>>12) & 0x1ff;
            flags=0b111;

            if(kernel_pml4[pdpt_index]==0)kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
            if(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]==0)((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)alloc_page(1) | flags;
            if(((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]==0)((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);

            for(uint64 i=framebuffer_start_pageframe;i<framebuffer_start_pageframe+framebuffer_size_page;i++){
                ((uint64*)(((uint64)((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]) & PAGE_ADDR_MASK))[i%512] = i*4096 | flags;
                if((i+1)%512==0){
                    pt_index++;
                    if(pt_index!=512){
                        ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)alloc_page(1) | flags;
                    }else{
                        pd_index++;
                        pt_index=0;
                        if(pd_index!=512){
                            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)((uint64)alloc_page(1) | flags);
                            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                        }
                        else {
                            pdpt_index++;
                            pd_index=0;
                            kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
                            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)alloc_page(1) | flags;
                            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                        }
                    }
                }
            }
            
            //for(uint64 i=0;i<width*height;i++) framebuffer[i]=0x00ff0000;

            /*uint8 t[]="ASD";
            for(uint32 i=0;i<3;i++){
                outb(0x3f8, t[i]);
            }*/
            //while(1);
            flags=0b111; //user
            pdpt_index=KERNEL_START_PDPT;
            pd_index=KERNEL_START_PD;
            pt_index=KERNEL_START_PT;

            /*kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)((uint64)alloc_page(1) | flags);
            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
            uint64* pt=(uint64*)((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index];*/
            if(kernel_pml4[pdpt_index]==0)kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
            if(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]==0)((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)alloc_page(1) | flags;
            if(((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]==0)((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
            
            uint32 kernel_size_page=kernelfilesize/4096+1;
            void* kernel=alloc_page(kernel_size_page);
            for(uint32 i=0;i<kernel_size_page*4096;i++){
                *(uint8*)(kernel+i)=*(uint8*)(kernel_file+i);
            }
            i=0;
            for(i=0;i<kernel_size_page;i++){
                ((uint64*)(((uint64)((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]) & PAGE_ADDR_MASK))[i%512] = ((uint64)kernel + i*4096) | flags;
                if((i+1)%512==0){
                    pt_index++;
                    if(pt_index!=512){
                        ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)alloc_page(1) | flags;
                    }else{
                        pd_index++;
                        pt_index=0;
                        if(pd_index!=512){
                            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)((uint64)alloc_page(1) | flags);
                            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                        }
                        else {
                            pdpt_index++;
                            pd_index=0;
                            kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
                            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)alloc_page(1) | flags;
                            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                        }
                    }
                }
            }

            kernel_info* kernel_info= alloc_page(1);
            uint64 kernel_info_vma=(pdpt_index<<39) | (pd_index<<30) | (pt_index << 21) | (i << 12);

            ((uint64*)(((uint64)((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]) & PAGE_ADDR_MASK))[i%512] = ((uint64)kernel_info) | flags;
            if((i+1)%512==0){
                pt_index++;
                if(pt_index!=512){
                    ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)alloc_page(1) | flags;
                }else{
                    pd_index++;
                    pt_index=0;
                    if(pd_index!=512){
                        ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)((uint64)alloc_page(1) | flags);
                        ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                    }
                    else {
                        pdpt_index++;
                        pd_index=0;
                        kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
                        ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)alloc_page(1) | flags;
                        ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                    }
                }
            }
            i++;

            

            void* kernel_stack = alloc_page(25);
            for(uint32 k=0;k<25;k++){
                ((uint64*)(((uint64)((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]) & PAGE_ADDR_MASK))[i%512] = ((uint64)kernel_stack + k*4096) | flags;
                if((i+1)%512==0){
                    pt_index++;
                    if(pt_index!=512){
                        ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)alloc_page(1) | flags;
                    }else{
                        pd_index++;
                        pt_index=0;
                        if(pd_index!=512){
                            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)((uint64)alloc_page(1) | flags);
                            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                        }
                        else {
                            pdpt_index++;
                            pd_index=0;
                            kernel_pml4[pdpt_index]=(uint64***)((uint64)alloc_page(1) | flags);
                            ((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index]=(uint64)alloc_page(1) | flags;
                            ((uint64*)((uint64)(((uint64*)((uint64)kernel_pml4[pdpt_index] & PAGE_ADDR_MASK))[pd_index])& PAGE_ADDR_MASK)) [pt_index]=(uint64)((uint64)alloc_page(1) | flags);
                        }
                    }
                }
                i++;
            }
            uint64 kernel_stack_vma=(pdpt_index<<39) | (pd_index<<30) | (pt_index << 21) | (i << 12)-1;

            load_pml4(kernel_pml4);

            kernel_info->frame_buffer=(uint64)framebuffer;
            kernel_info->kernel_pml4_addresss=(uint64)kernel_pml4;
            kernel_info->screen_width=(uint64)width;
            kernel_info->screen_height=(uint64)height;
            kernel_info->real_address=(uint64)kernel;
            kernel_info->largest_area_idx=best_mem_part_idx;
            kernel_info->memmap_desc_size=MemMapDescriptorSize;
            kernel_info->memmap_desc_count=DescriptorCount;
            kernel_info->memmap=memmap;
            kernel_info->used_pages=page_idx;
            kernel_info->gdt=&gdt_table;
            kernel_info->tss=&tss;
            kernel_info->kernel_next_page=i;
            
            uint64 kernel_virt_addr=/*offset*/(uint64)0 | (uint64)0<<12/*p_idx*/ | pt_index<<21 | pd_index<<30 | pdpt_index<<39;
            jmp_to_kernel((void*)kernel_stack_vma, (void*)kernel_virt_addr,(void*)kernel_info_vma);
        }
    }
    while(1);
return EFI_SUCCESS;
}