#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

/* This file contains all of the structures and definitions
 * required to compatibly access the PCI bus,
 *  as well as set up new PCI devices, PCI bridges, and manipulate
 *   the connections of PCI lanes.
 */

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC


const char* PCIGetDeviceName_Subclass(uint8_t DeviceClass, uint8_t Subclass, uint8_t ProgrammableInterface);

const char* PCIGetClassName(uint8_t DeviceClass);

void PCIEnumerate();

uint32_t PCIReadConfig(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);

typedef struct __attribute__((packed)) {
    uint8_t IOMapped : 1;                    // Device can respond to I/O access
    uint8_t MMIOMapped : 1;                  // Device can respond to memory access (device is MMIO mapped)
    uint8_t IsBusMaster : 1;                 // Device is Bus Master; can generate PCI addresses
    uint8_t MonitorSpecialCycle : 1;         // Device can monitor Special Cycle
    uint8_t AllowMemWriteInvalidate : 1;     // Device can generate Memory Write And Invalidate commands; else Memory Write must be used
    uint8_t SnoopVGA : 1;                    // Device snoops the VGA palette on write; else is treated like a normal access
    uint8_t RespondsParityError : 1;         // Device responds to Parity Errors by setting PERR#; else will set pci_status#ParityError and continue.
    uint8_t _reserved : 1;                   // Hardwired to 0
    uint8_t SystemErr : 1;                   // Enable SERR# driver; System ERRor
    uint8_t FastBack2Back : 1;               // Device is allowed to generate fast back-to-back transactions to other agents.
    uint8_t DisableInterrupt : 1;            // Disable assertion of INTx# signal; else enable.
} pci_command_t;

typedef struct __attribute__((packed)) {
    uint8_t _reserved : 3;                   // 3 bits hardwired to 0   
    uint8_t InterruptActive : 1;             // State of device's INTx# signal. If pci_command#DisableInterrupt is 0 and this is 1, the signal will be asserted.
    uint8_t Capabilities : 1;                // If set, device will implement New Capabilities list at 0x34 offset.
    uint8_t Mhz66Capable : 1;                // Device can run at 66MHz. Else, device will run at 33MHz.
    uint8_t _reserved1 : 1;                  // Reserved as of 3.0, Used in 2.1 as "Supports User Definable Features"
    uint8_t FastBack2Back : 1;               // Device is allowed to accept fast back-to-back transactions from other agents.
    uint8_t MasterParityError : 1;           // Only set when PERR# is asserted by the Bus Master while pci_command#RespondsParityError is set.
    uint8_t DEVSELTiming : 2;                // Read-Only; represents the slowest time a device will assert DEVSEL#. 00 = fast, 01 = medium, 11 = slow.
    uint8_t TargetAborted : 1;               // Target device terminated transaction via Target-Abort
    uint8_t ReceivedTargetAbort : 1;         // Master's connection was terminated by Target-Abort
    uint8_t ReceivedMasterAbort : 1;         // Master's connection was terminated by Master-Abort
    uint8_t SystemAssertedError : 1;         // Device asserted SERR#
    uint8_t ParityError : 1;                 // Device detected parity error. Parity error may not be handled.
} pci_status_t;


typedef struct __attribute__((packed)) {
    uint16_t      VendorID;                  // 16 bit Vendor ID allocated by PCI-SIG. 0xFFFF is invalid.
    uint16_t      DeviceID;                  // 16 bit Device ID allocated by the vendor.
    pci_command_t Command;                   // 16 bit PCI_COMMAND register.
    pci_status_t  Status;                    // 16 bit PCI_STATUS register.
    uint8_t       RevisionID;                // 8 bit register, revision identifier specified by vendor.
    uint8_t       ProgrammingInterface;      // 8 bit register, identifies any programming interface the device may have.
    uint8_t       Subclass;                  // 8 bit Subclass code; identifies the specific function of the device
    uint8_t       ClassCode;                 // 8 bit Class Code; identifies the function of the device
    uint8_t       CacheLine;                 // 8 bit; specifies system cache line size in 32-bit blocks. Devices can limit this. Unsupported values are treated as 0.
    uint8_t       LatencyTimer;              // 8 bit; specifies latency timer in (bus clock) units.
    uint8_t       HeaderType;                // 8 bit; identifies the layout of the header and the type of device; 00 = device, 01 = pci-pci bridge, 11 = CardBus bridge. Multi-function defined by bit 7.
    uint8_t       BuiltInSelfTest;           // 8 bit; status and control of a device's built-in self-test (BIST)
} pci_header_common_t;

typedef struct __attribute__((packed)) {
    // pci_header_common_t first, then..
    uint32_t BARs[6];                        // 6 x 32 bit Base Address Registers (BARs)
    uint32_t CISPtr;                         // Points to Card Information Structure for PCI devices that share silicon with CardBus
    uint16_t SubsysVendor;              
    uint16_t SubsysID;
    uint32_t ExpansionBAR;                   // Points to the base of an Expansion ROM.
    uint8_t  Capabilities;                   // The pointer generated by pci_status#capabilities_list
    uint8_t  _reserved[3];                   // 24 bit reserved at top end of register.
    uint32_t _reserved2;
    uint8_t  InterruptPin;                   // Specifies the PIC pin that INTx# is connected to. Can be 0-15 because x86 PICs have 16 IRQs. 0xFF is no connection.
    uint8_t  InterruptActive;                // Specifies the interrupt pin the device uses. 0x1 is INTA#, 0x2 is INTB#, 0x3 is INTC#, 0x4 is INTD#, 0x0 is no interrupt.
    uint8_t  MinimumTimeshare;               // Specifies the length of the burst period, in quarter-microsecond units
    uint8_t  MaxmimumTimeshare;              // Specifies how often the device accesses the PCI bus - in quarter-microseconds
} pci_header_device_t;

typedef struct __attribute__((packed)) {
    // pci_header_common_t first
    uint32_t     BARs[2];
    uint8_t      PrimaryBus;                 // Primary Bus Number.
    uint8_t      SecondaryBus;               // Secondary Bus Number.
    uint8_t      SubordinateBus;             // Subordinate Bus Number.
    uint8_t      LatencyTimer;               // Secondary Latency Timer.
    uint8_t      IOBaseLow;                  // IO Base is 24 bits. This is lower 8.
    uint8_t      IOLimitLow;                 // IO Limit is 24 bits. This is lower 8.
    pci_status_t SecondaryStatus;            // Secondary Status.
    uint16_t     MemoryBase;
    uint16_t     MemoryLimit;
    uint16_t     PrefetchBaseLow;            // Prefetchable Memory Base is 48 bits. This is lower 16.
    uint16_t     PrefetchLimitLow;           // Prefetchable Memory Limit is 48 bits. This is lower 16.
    uint32_t     PrefetchBaseHigh;           // Prefetchable Memory Base is 48 bits. This is upper 32.
    uint32_t     PrefetchLimitHigh;          // Prefetchable Memory Limit is 48 bits. This is upper 32.
    uint16_t     IOBaseHigh;                 // IO Base is 24 bits. This is upper 16.
    uint16_t     IOLimitHigh;                // IO Limit is 24 bits. This is upper 16.
    uint8_t      Capabilities;               // Pointer generated by pci_status#capabilities_list
    uint8_t      _reserved[3];               // 24 reserved bits.
    uint32_t     ExpansionBAR;               // Base of Expansion ROM.
    uint8_t      InterruptPin;               // Specifies the PIC pin that INTx# is connected to. Can be 0-15 because x86 PICs have 16 IRQs. 0xFF is no connection.
    uint8_t      InterruptActive;            // Specifies the interrupt pin the device uses. 0x1 is INTA#, 0x2 is INTB#, 0x3 is INTC#, 0x4 is INTD#, 0x0 is no interrupt.
    uint16_t     BridgeControl;            
} pci_header_bridge_t;


typedef struct {
    uint16_t    segment;
    uint8_t     bus;
    uint8_t     slot;
    uint8_t     function;
} pci_address_t;

typedef struct {
    uint8_t Present : 1;
    uint8_t MMIO : 1;

    union {
        size_t Address;
        uint16_t Port;
    };
    
    size_t Length;
} pci_bar_t;

typedef struct pci_device {
    
    struct pci_dev* Parent; // Parent of the device (for PCI hubs / splitters)

    uint16_t DeviceID;
    uint16_t VendorID;

    uint8_t DeviceClass;
    uint8_t Subclass;

    uint8_t ProgrammableInterface;

    pci_address_t address;

    pci_bar_t BARs[6];

    int IRQ; // The IRQ of the device if already handled

    // The headers!
    volatile pci_header_common_t* Header;

    union { // The device can only be one of these at a time, but they both form part of the config space.
        volatile pci_header_bridge_t* bridge;
        volatile pci_header_device_t* device;
    };

    struct pci_dev** Children; // If this device is a hub, it has children...

    // acpi_node_t acpi;
} pci_device_t;

typedef struct {
    pci_address_t key;
    pci_device_t* value;
} pci_entry_t;


extern pci_device_t** pci_root_devices;
extern pci_entry_t* pci_map;

#ifdef  __cplusplus
}
#endif