#pragma once
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

extern pci_dev_t** pci_root_devices;
extern pci_entry_t* pci_map;

int pci_init_early();

const char* pci_get_name(uint8_t devclass, uint8_t subclass, uint8_t progif);

int pci_enumerate_devices(pci_dev_t* root);

int pci_get_config(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t function, void** config);

typedef struct __attribute__((packed)) {
    uint8_t io_space : 1;                    // Device can respond to I/O access
    uint8_t memory_space : 1;                // Device can respond to memory access (device is MMIO mapped)
    uint8_t bus_master : 1;                  // Device is Bus Master; can generate PCI addresses
    uint8_t special_cycle : 1;               // Device can monitor Special Cycle
    uint8_t memory_write_and_invalidate : 1; // Device can generate Memory Write And Invalidate commands; else Memory Write must be used
    uint8_t vga_palette : 1;                 // Device snoops the VGA palette on write; else is treated like a normal access
    uint8_t parity_error_response : 1;       // Device responds to Parity Errors by setting PERR#; else will set pci_status#parity_error and continue.
    uint8_t _reserved : 1;                   // Hardwired to 0
    uint8_t serr : 1;                        // Enable SERR# driver; System ERRor
    uint8_t fast_back_back : 1;              // Device is allowed to generate fast back-to-back transactions to other agents.
    uint8_t disable_interrupt : 1;           // Disable assertion of INTx# signal; else enable.
} pci_command_t;

typedef struct __attribute__((packed)) {
    uint8_t _reserved : 3;                   // 3 bits hardwired to 0   
    uint8_t interrupt : 1;                   // State of device's INTx# signal. If pci_command#disable_interrupt is 0 and this is 1, the signal will be asserted.
    uint8_t capability_list : 1;             // If set, device will implement New Capabilities list at 0x34 offset.
    uint8_t freq_66_capable : 1;             // Device can run at 66MHz. Else, device will run at 33MHz.
    uint8_t _reserved1 : 1;                  // Reserved as of 3.0, Used in 2.1 as "Supports User Definable Features"
    uint8_t fast_back_back : 1;              // Device is allowed to accept fast back-to-back transactions from other agents.
    uint8_t master_parity_error : 1;         // Only set when PERR# is asserted by the Bus Master while pci_command#parity_error_response is set.
    uint8_t devsel_timing : 2;               // Read-Only; represents the slowest time a device will assert DEVSEL#. 00 = fast, 01 = medium, 11 = slow.
    uint8_t target_signalled_abort : 1;      // Target device terminated transaction via Target-Abort
    uint8_t received_target_abort : 1;       // Master's connection was terminated by Target-Abort
    uint8_t received_master_abort : 1;       // Master's connection was terminated by Master-Abort
    uint8_t system_error_asserted : 1;       // Device asserted SERR#
    uint8_t parity_error : 1;                // Device detected parity error. Parity error may not be handled.
} pci_status_t;


typedef struct __attribute__((packed)) {
    uint16_t      vendor_id;                 // 16 bit Vendor ID allocated by PCI-SIG. 0xFFFF is invalid.
    uint16_t      device_id;                 // 16 bit Device ID allocated by the vendor.
    pci_command_t command;                   // 16 bit PCI_COMMAND register.
    pci_status_t  status;                    // 16 bit PCI_STATUS register.
    uint8_t       revision_id;               // 8 bit register, revision identifier specified by vendor.
    uint8_t       progIF;                    // 8 bit register, identifies any programming interface the device may have.
    uint8_t       subclass;                  // 8 bit Subclass code; identifies the specific function of the device
    uint8_t       class_code;                // 8 bit Class Code; identifies the function of the device
    uint8_t       cache_line_size;           // 8 bit; specifies system cache line size in 32-bit blocks. Devices can limit this. Unsupported values are treated as 0.
    uint8_t       latency_timer;             // 8 bit; specifies latency timer in (bus clock) units.
    uint8_t       header_type;               // 8 bit; identifies the layout of the header and the type of device; 00 = device, 01 = pci-pci bridge, 11 = CardBus bridge. Multi-function defined by bit 7.
    uint8_t       bist;                      // 8 bit; status and control of a device's built-in self-test (BIST)
} pci_header_common_t;

typedef struct __attribute__((packed)) {
    // pci_header_common_t first, then..
    uint32_t bar[6];                         // 6 x 32 bit Base Address Registers (BARs)
    uint32_t cis_pointer;                    // Points to Card Information Structure for PCI devices that share silicon with CardBus
    uint16_t subsystem_vendor_id;              
    uint16_t subsystem_id;
    uint32_t expansion_bar;                  // Points to the base of an Expansion ROM.
    uint8_t  capabilities;                   // The pointer generated by pci_status#capabilities_list
    uint8_t  _reserved[3];                   // 24 bit reserved at top end of register.
    uint32_t _reserved2;
    uint8_t  interrupt_line;                  // Specifies the PIC pin that INTx# is connected to. Can be 0-15 because x86 PICs have 16 IRQs. 0xFF is no connection.
    uint8_t  interrupt;                      // Specifies the interrupt pin the device uses. 0x1 is INTA#, 0x2 is INTB#, 0x3 is INTC#, 0x4 is INTD#, 0x0 is no interrupt.
    uint8_t  min_grant;                      // Specifies the length of the burst period, in quarter-microsecond units
    uint8_t  max_latency;                    // Specifies how often the device accesses the PCI bus - in quarter-microseconds
} pci_header_device_t;

typedef struct __attribute__((packed)) {
    // pci_header_common_t first
    uint32_t     bar[2];
    uint8_t      pri_bus;                    // Primary Bus Number.
    uint8_t      sec_bus;                    // Secondary Bus Number.
    uint8_t      sub_bus;                    // Subordinate Bus Number.
    uint8_t      sec_latency_timer;          // Secondary Latency Timer.
    uint8_t      io_base;                    // IO Base is 24 bits. This is lower 8.
    uint8_t      io_limit;                   // IO Limit is 24 bits. This is lower 8.
    pci_status_t sec_status;                 // Secondary Status.
    uint16_t     mem_base;
    uint16_t     mem_limit;
    uint16_t     mem_base_prefetch;          // Prefetchable Memory Base is 48 bits. This is lower 16.
    uint16_t     mem_limit_prefetch;         // Prefetchable Memory Limit is 48 bits. This is lower 16.
    uint32_t     mem_base_prefetch_upper;    // Prefetchable Memory Base is 48 bits. This is upper 32.
    uint32_t     mem_limit_prefetch_upper;   // Prefetchable Memory Limit is 48 bits. This is upper 32.
    uint16_t     io_base_upper;              // IO Base is 24 bits. This is upper 16.
    uint16_t     io_limit_upper;             // IO Limit is 24 bits. This is upper 16.
    uint8_t      capabilities;               // Pointer generated by pci_status#capabilities_list
    uint8_t      _reserved[3];               // 24 reserved bits.
    uint32_t     expansion_bar;              // Base of Expansion ROM.
    uint8_t      interrupt_line;             // Specifies the PIC pin that INTx# is connected to. Can be 0-15 because x86 PICs have 16 IRQs. 0xFF is no connection.
    uint8_t      interrupt;                  // Specifies the interrupt pin the device uses. 0x1 is INTA#, 0x2 is INTB#, 0x3 is INTC#, 0x4 is INTD#, 0x0 is no interrupt.
    uint16_t     bridge_control;            
} pci_header_bridge_t;


typedef struct {
    uint16_t    segment;
    uint8_t     bus;
    uint8_t     slot;
    uint8_t     function;
} pci_address_t;

typedef struct {
    uint8_t present : 1;
    uint8_t mmio : 1;

    union {
        size_t addr;
        uint16_t port;
    };
    size_t length;
} pci_bar_t;

typedef struct pci_dev{
    
    struct pci_dev* parent; // Parent of the device (for PCI hubs / splitters)

    uint16_t device_id;
    uint16_t vendor_id;

    uint8_t devclass;
    uint8_t subclass;

    uint8_t progif;

    pci_address_t address;

    pci_bar_t bars[6];

    int irq; // The IRQ of the device if already handled

    // The headers!
    volatile pci_header_common_t* header;

    union { // The device can only be one of these at a time, but they both form part of the config space.
        volatile pci_header_bridge_t* bridge;
        volatile pci_header_device_t* device;
    };

    struct pci_dev** children; // If this device is a hub, it has children...

    // acpi_node_t acpi;
} pci_dev_t;

typedef struct {
    pci_address_t key;
    pci_dev_t* value;
} pci_entry_t;


