#include <acpi.h>
#include <basic_functions.h>

void init_devices(RSDPDescriptor20* rsdp2){
    char signature[] = "RSD PTR ";
    if(!memcmp(signature,(char*)(rsdp2->firstPart.Signature),8))
        kernel_panic("ACPI RDST PTR signature not valid.");
    if(!verify_rsdp_checksum(rsdp2))
        kernel_panic("ACPI RSDT PTR checksum failed to verify.");
    
    #if(ACPI_DEBUG)

        print_to_serial("ACPI RSDT PTR verified\n\r OEMID: ");
        for(uint32 i=0;i<6;i++)outb(SERIAL_PORT,rsdp2->firstPart.OEMID[i]);
        print_to_serial("\n\r");

    #endif

    xsdt=rsdp2->XsdtAddress;

    if(!acpi_sdt_checksum(&(xsdt->header)))
        kernel_panic("ACPI XSDT Checksum failed to ferify.");
    
    #if(ACPI_DEBUG)

        print_to_serial("ACPI XSDT verified\n\r OEMID: ");
        for(uint32 i=0;i<6;i++)outb(SERIAL_PORT,xsdt->header.OEMID[i]);
        print_to_serial("\n\r");

    #endif

    uint32 count=(xsdt->header.Length - sizeof(xsdt->header))/8;

    #if(ACPI_DEBUG)

        print_to_serial("ACPI tables:\n\r");
        print_to_serial("|Sign|Address\n\r");
        for(uint32 i=0;i<count;i++){
            ACPISDTHeader* table = (ACPISDTHeader*)xsdt->addresses[i];
            print_to_serial("|");
            for(uint32 j=0;j<4;j++)
                outb(SERIAL_PORT,table->Signature[j]);
            print_to_serial("|");
            print_hex_to_serial((uint64)table);
            print_to_serial("\n\r");
        }

    #endif

    for(uint32 i=0;i<count;i++){
        ACPISDTHeader* table = (ACPISDTHeader*)xsdt->addresses[i];
        if(memcmp(table->Signature,"FACP",4)){
            fadt=(FADT*)table;
            acpi_process_fadt();
        }
    }

}

uint8 verify_rsdp_checksum(RSDPDescriptor20* rsdp2){
    uint8* first_part=(uint8*)(&(rsdp2->firstPart));
    uint8 chsum=0;

    for(uint32 i=0;i<sizeof(rsdp2->firstPart);i++){
        chsum+=first_part[i];
    }
    if(chsum!=0)return 0;

    uint8* second_part=first_part+sizeof(rsdp2->firstPart);
    for(uint32 i=0;i<sizeof(*rsdp2) - sizeof(rsdp2->firstPart);i++){
        chsum+=second_part[i];
    }

    /*print_to_serial("Checksum: ");
    print_int_to_serial(chsum);
    print_to_serial("\n\r");
    print_to_serial("sizeof(RSDPDescriptor20): ");
    print_int_to_serial(sizeof(RSDPDescriptor20));
    print_to_serial("\n\r");*/

    if(chsum!=0)return 0;
    return 1;
}

uint8 acpi_sdt_checksum(ACPISDTHeader* header){
    uint8 checksum=0;
    uint8* pointer=(uint8*)header;
    for(uint32 i=0;i<header->Length;i++){
        checksum+=pointer[i];
    }
    return checksum==0;
}

void acpi_process_fadt(){
    if(!acpi_sdt_checksum(&(fadt->header)))
        kernel_panic("ACPI FADT checksum failed to verify.");

    #if(ACPI_DEBUG)
        print_to_serial("ACPI FADT successfully verified.\n\r");
    #endif

    if(fadt->Dsdt!=0)
        dsdt=(DSDT*)(fadt->Dsdt);
    else
        dsdt=(DSDT*)(fadt->X_Dsdt);
    
    if(fadt->FirmwareCtrl!=0)
        facs=fadt->FirmwareCtrl;
    else
        facs=fadt->X_FirmwareControl;

    
}

void acpi_process_dsdt(){
    if(!acpi_sdt_checksum(&(dsdt->header)))
        kernel_panic("ACPI DSDT failed to verify.");

    #if(ACPI_DEBUG)
        print_to_serial("ACPI DSDT successfully verified.\n\r");
    #endif

    
}