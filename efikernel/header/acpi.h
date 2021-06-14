#ifndef _ACPI_H
#define _ACPI_H
#include <types.h>
#include <kernel_main.h>
#include <config.h>

#pragma pack(1)

typedef enum __attribute__ ((__packed__)) AcpiAddressSpace {
    AcpiSystemMemory                                = 0,
    AcpiSystemIO                                    = 1,
    AcpiPciConfigSpace                              = 2,
    AcpiEmbeddedController                          = 3,
    AcpiSystemManagementBus                         = 4,
    AcpiSystemCmos                                  = 5,
    AcpiPciDeviceBarTarget                          = 6,
    AcpiIntelligentPlatformManagementInfrastructure = 7,
    AcpiGeneralPurposeIO                            = 8,
    AcpiGenericSerialBus                            = 9,
    AcpiPlatformCommunicationChannel                = 10
} AcpiAddressSpace;

typedef enum __attribute__ ((__packed__)) AcpiAccessSize {
    AcpiAccessSizeUndefined = 0,
    AcpiAccessSizeByte      = 1,
    AcpiAccessSizeWord      = 2,
    AcpiAccessSizeDword     = 3,
    AcpiAccessSizeQword     = 4
} AcpiAccessSize;

typedef enum __attribute__ ((__packed__)) AcpiPreferredPowerManagementProfile{
    AcpiPowerProfileUnspecified         = 0,
    AcpiPowerProfileDesktop             = 1,
    AcpiPowerProfileMobile              = 2,
    AcpiPowerProfileWorkstation         = 3,
    AcpiPowerProfileEnterpriseServer    = 4,
    AcpiPowerProfileSohoServer          = 5,
    AcpiPowerProfileAppliancePc         = 6,
    AcpiPowerProfilePerformanceServer   = 7
} AcpiPreferredPowerManagementProfile;

typedef struct GenericAddressStructure
{
    AcpiAddressSpace AddressSpace;
    uint8 BitWidth;
    uint8 BitOffset;
    AcpiAccessSize AccessSize;
    uint64 Address;
} GenericAddressStructure;

typedef struct ACPISDTHeader {
    uint8 Signature[4];
    uint32 Length;
    uint8 Revision;
    uint8 Checksum;
    uint8 OEMID[6];
    uint8 OEMTableID[8];
    uint32 OEMRevision;
    uint32 CreatorID;
    uint32 CreatorRevision;
} ACPISDTHeader;

typedef struct XSDT {
    ACPISDTHeader header;
    uint64 addresses[];
} XSDT;

typedef struct DSDT {
    ACPISDTHeader header;
} DSDT;

typedef struct RSDPDescriptor {
    uint8 Signature[8];
    uint8 Checksum;
    uint8 OEMID[6];
    uint8 Revision;
    uint32 RsdtAddress;
} RSDPDescriptor;

typedef struct RSDPDescriptor20 {
    RSDPDescriptor firstPart;
    uint32 Length;
    XSDT* XsdtAddress;
    uint8 ExtendedChecksum;
    uint8 reserved[3];
} RSDPDescriptor20;

typedef struct FADT // From osdev
{
    ACPISDTHeader header;
    uint32 FirmwareCtrl;
    uint32 Dsdt;
 
    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8  Reserved;
 
    AcpiPreferredPowerManagementProfile  PreferredPowerManagementProfile;
    uint16 SCI_Interrupt;
    uint32 SMI_CommandPort;
    uint8  AcpiEnable;
    uint8  AcpiDisable;
    uint8  S4BIOS_REQ;
    uint8  PSTATE_Control;
    uint32 PM1aEventBlock;
    uint32 PM1bEventBlock;
    uint32 PM1aControlBlock;
    uint32 PM1bControlBlock;
    uint32 PM2ControlBlock;
    uint32 PMTimerBlock;
    uint32 GPE0Block;
    uint32 GPE1Block;
    uint8  PM1EventLength;
    uint8  PM1ControlLength;
    uint8  PM2ControlLength;
    uint8  PMTimerLength;
    uint8  GPE0Length;
    uint8  GPE1Length;
    uint8  GPE1Base;
    uint8  CStateControl;
    uint16 WorstC2Latency;
    uint16 WorstC3Latency;
    uint16 FlushSize;
    uint16 FlushStride;
    uint8  DutyOffset;
    uint8  DutyWidth;
    uint8  DayAlarm;
    uint8  MonthAlarm;
    uint8  Century;
 
    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16 BootArchitectureFlags;
 
    uint8  Reserved2;
    uint32 Flags;
 
    GenericAddressStructure ResetReg;
 
    uint8  ResetValue;
    uint8  Reserved3[3];
 
    // 64bit pointers - Available on ACPI 2.0+
    uint64                X_FirmwareControl;
    uint64                X_Dsdt;
 
    GenericAddressStructure X_PM1aEventBlock;
    GenericAddressStructure X_PM1bEventBlock;
    GenericAddressStructure X_PM1aControlBlock;
    GenericAddressStructure X_PM1bControlBlock;
    GenericAddressStructure X_PM2ControlBlock;
    GenericAddressStructure X_PMTimerBlock;
    GenericAddressStructure X_GPE0Block;
    GenericAddressStructure X_GPE1Block;
} FADT;

#pragma pack()

XSDT* xsdt;
FADT* fadt;
uint64 facs;
DSDT* dsdt;

void init_devices(RSDPDescriptor20* rsdp2);
uint8 verify_rsdp_checksum(RSDPDescriptor20* rsdp2);
uint8 acpi_sdt_checksum(ACPISDTHeader* header);
void acpi_process_fadt();

#endif