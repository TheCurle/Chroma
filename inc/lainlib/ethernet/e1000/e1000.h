/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

#include <stdint.h>
#include <stdbool.h>
#include <kernel/system/pci.h>
#include <kernel/system/interrupts.h>
#include <kernel/system/io.h>

#define INTEL_VEND     0x8086  // Vendor ID for Intel 
#define E1000_DEV      0x100E  // Device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated NICs
#define E1000_I217     0x153A  // Device ID for Intel I217
#define E1000_82577LM  0x10EA  // Device ID for Intel 82577LM
 
#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_IMASK       0x00D0
#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818
 
#define REG_TCTRL       0x0400
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818
 
 
#define REG_RDTR         0x2820 // RX Delay Timer Register
#define REG_RXDCTL       0x3828 // RX Descriptor Control
#define REG_RADV         0x282C // RX Int. Absolute Delay Timer
#define REG_RSRPD        0x2C00 // RX Small Packet Detect Interrupt
 
#define REG_MAC          0x5400 // MAC address base
 
#define REG_TIPG         0x0410      // Transmit Inter Packet Gap
#define ECTRL_SLU        0x40        // set-link-up
 
 
#define RCTL_EN                         (1 << 1)    // Receiver Enable
#define RCTL_SBP                        (1 << 2)    // Store Bad Packets
#define RCTL_UPE                        (1 << 3)    // Unicast Promiscuous Enabled
#define RCTL_MPE                        (1 << 4)    // Multicast Promiscuous Enabled
#define RCTL_LPE                        (1 << 5)    // Long Packet Reception Enable
#define RCTL_LBM_NONE                   (0 << 6)    // No Loopback
#define RCTL_LBM_PHY                    (3 << 6)    // PHY or external SerDesc loopback
#define RCTL_RDMTS_HALF                 (0 << 8)    // Free Buffer Threshold is 1/2 of RDLEN
#define RCTL_RDMTS_QUARTER              (1 << 8)    // Free Buffer Threshold is 1/4 of RDLEN
#define RCTL_RDMTS_EIGHTH               (2 << 8)    // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36                      (0 << 12)   // Multicast Offset - bits 47:36
#define RCTL_MO_35                      (1 << 12)   // Multicast Offset - bits 46:35
#define RCTL_MO_34                      (2 << 12)   // Multicast Offset - bits 45:34
#define RCTL_MO_32                      (3 << 12)   // Multicast Offset - bits 43:32
#define RCTL_BAM                        (1 << 15)   // Broadcast Accept Mode
#define RCTL_VFE                        (1 << 18)   // VLAN Filter Enable
#define RCTL_CFIEN                      (1 << 19)   // Canonical Form Indicator Enable
#define RCTL_CFI                        (1 << 20)   // Canonical Form Indicator Bit Value
#define RCTL_DPF                        (1 << 22)   // Discard Pause Frames
#define RCTL_PMCF                       (1 << 23)   // Pass MAC Control Frames
#define RCTL_SECRC                      (1 << 26)   // Strip Ethernet CRC
 
// Buffer Sizes
#define RCTL_BSIZE_256                  (3 << 16)
#define RCTL_BSIZE_512                  (2 << 16)
#define RCTL_BSIZE_1024                 (1 << 16)
#define RCTL_BSIZE_2048                 (0 << 16)
#define RCTL_BSIZE_4096                 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192                 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384                ((1 << 16) | (1 << 25))
 
 
// Transmit Command
 
#define CMD_EOP                         (1 << 0)    // End of Packet
#define CMD_IFCS                        (1 << 1)    // Insert FCS
#define CMD_IC                          (1 << 2)    // Insert Checksum
#define CMD_RS                          (1 << 3)    // Report Status
#define CMD_RPS                         (1 << 4)    // Report Packet Sent
#define CMD_VLE                         (1 << 6)    // VLAN Packet Enable
#define CMD_IDE                         (1 << 7)    // Interrupt Delay Enable
 
 
// TCTL Register
 
#define TCTL_EN                         (1 << 1)    // Transmit Enable
#define TCTL_PSP                        (1 << 3)    // Pad Short Packets
#define TCTL_CT_SHIFT                   4           // Collision Threshold
#define TCTL_COLD_SHIFT                 12          // Collision Distance
#define TCTL_RRTHRES_SHIFT              29          // Read Request Threshold
#define TCTL_SWXOFF                     (1 << 22)   // Software XOFF Transmission
#define TCTL_RTLC                       (1 << 24)   // Re-transmit on Late Collision
 
#define TSTA_DD                         (1 << 0)    // Descriptor Done
#define TSTA_EC                         (1 << 1)    // Excess Collisions
#define TSTA_LC                         (1 << 2)    // Late Collision
#define LSTA_TU                         (1 << 3)    // Transmit Underrun

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8
 
struct e1000_receive_packet {
        volatile uint64_t Address;
        volatile uint16_t Length;
        volatile uint16_t checksum;
        volatile uint8_t Status;
        volatile uint8_t errors;
        volatile uint16_t Special;
} __attribute__((packed));
 
struct e1000_transmit_packet {
        volatile uint64_t Address;
        volatile uint16_t Length;
        volatile uint8_t CSO;
        volatile uint8_t Command;
        volatile uint8_t Status;
        volatile uint8_t CSS;
        volatile uint16_t Special;
} __attribute__((packed));

typedef struct e1000_device {
    // BAR0's type
    uint8_t BARType;
    // The base IO address
    uint16_t IOBase;
    // The base MMIO address
    uint64_t  MemoryBase;   // MMIO Base Address
    // Does this card use an EEPROM?
    bool HasEEPROM;
    // The MAC address
    uint8_t MAC[6];
    // Receive circular buffer.
    struct e1000_receive_packet* ReceivePackets[E1000_NUM_RX_DESC];
    // Transmit circular buffer.
    struct e1000_transmit_packet* TransmitPackets[E1000_NUM_TX_DESC];
    // Current receive packet index
    uint16_t CurrentReceivePacket;
    // Current transmit packet index
    uint16_t CurrentTransmitPacket;
} e1000_device_t;

 
// Write a command to the E1000's registers
void E1000WriteCommandRegister(e1000_device_t* Device, uint16_t Address, uint32_t Data);
// Read the E1000's command registers
uint32_t E1000ReadCommandRegister(e1000_device_t* Device, uint16_t Address);
 
// Does this card use an EEPROM?
bool E1000DetectEEPROM(e1000_device_t* Device);
// Read 4 bytes from an EEPROM address
uint32_t E1000ReadEEPROM(e1000_device_t* Device, uint8_t Address);
// Read this NIC's MAC address
bool E1000ReadMAC(e1000_device_t* Device);
// Connect to a network, if there is one
void E1000Uplink(e1000_device_t* Device);
// Prepare for receiving packets
void E1000InitRX(e1000_device_t* Device);
// Prepare for sending packets
void E1000InitTX(e1000_device_t* Device);
// Prepare for receiving interrupts
void E1000InitInt(e1000_device_t* Device);
// Handle a received packet
void E1000Receive(e1000_device_t* Device);

// Handle constructing meta information about this device.
void E1000Init(e1000_device_t* Device, pci_device_t* PCIHeader);
// Initialise and start the driver
void E1000Startup(e1000_device_t* Device);
// Handle an interrupt received from the card
void E1000InterruptFired(InterruptFrame* InterruptContext);
// Get the E1000's MAC address
uint8_t* E1000GetMAC(e1000_device_t* Device);
// Send a packet
int E1000Send(e1000_device_t* Device, const void* Data, uint16_t Length);
