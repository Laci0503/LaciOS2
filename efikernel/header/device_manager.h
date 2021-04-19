#ifndef _DEVICE_MANAGER_H
#define _DEVICE_MANAGER_H
#include <types.h>

typedef enum device_type{
    device_serial_io,
    device_keyboard,
    device_mouse,
    device_pci_bus,
    device_storage,
    device_network
} device_type;

typedef struct pci_device_descriptor{
    uint32 bus;
    uint8 device;
    uint8 function;
    uint32 class;
    uint32 subclass;
    uint32 prog_if;
} pci_device_descriptor;

typedef struct device{
    uint64 id;
    device_type device_type;
    //uint64(*function)(uint64 instruction, void* args_pointer);
    uint64 parent;
    void* descriptor;
    void* driver;
} Device;

#endif