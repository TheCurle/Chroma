#include <lainlib/ethernet/e1000/e1000.h>
#include <kernel/chroma.h>
#include <kernel/system/memory.h>
#include <kernel/system/interrupts.h>

/*************************
 *** Team Kitty,  2021 ***
 ***     Lainlib       ***
 ************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This file handles all the logic for interfacing with the E1000 networking device.
 * This card is labelled either the Intel I217, or Intel Gigabit 82577LM.
 * These cards are identical, and this driver will work identically for both of them.
 *
 * To use this driver, allocate an e1000_device struct and pass it to the E1000Init() function,
 *  along with its' PCI device header.
 *
 * TODO: usage information
 */

/**
 * Write data to the device's command registers.
 * If we use BAR type 0, we use MMIO, otherwise ports.
 *
 * @param Device The device to which we write the data.
 * @param Address The address to write the data at. For MMIO, the offset from base.
 * @param Data The data to write into the register.
 */
void E1000WriteCommandRegister(e1000_device_t* Device, uint16_t Address, uint32_t Data) {
    if(Device->BARType == 0)
        WriteMMIO(Device->MemoryBase + Address, Data, 4);
    else {
        WritePort(Device->IOBase, Address, 4);
        WritePort(Device->IOBase + 4, Data, 4);
    }
}

/**
 * Read data from the device's command registers.
 * If we use BAR type 0, we read from MMIO. Otherwise, ports.
 * 
 * @param Device The device to read the data from
 * @param Address The address we expect the data to be at. For MMIO, the offset from base.
 * @return uint32_t The data contained in the register.
 */
uint32_t E1000ReadCommandRegister(e1000_device_t* Device, uint16_t Address) {
    if(Device->BARType == 0) 
        return ReadMMIO(Device->MemoryBase + Address, 4);
    else {
        WritePort(Device->IOBase, Address, 4);
        return ReadPort(Device->IOBase + 4, 4);
    }
}

/**
 * Attempt to detect the presence of an EEPROM in the E1000.
 * It sometimes doesn't like revealing its secrets, so we try it around 1000 times.
 * 
 * @param Device The device to attempt to detect an EEPROM inside.
 * @return true The given device has an EEPROM
 * @return false The given device does not have an EEPROM
 */
bool E1000DetectEEPROM(e1000_device_t* Device) {
    uint32_t Res = 0;
    E1000WriteCommandRegister(Device, REG_EEPROM, 0x1);
    Device->HasEEPROM = false;

    for(size_t i = 0; i < 1000 && !Device->HasEEPROM; i++) {
        Res = E1000ReadCommandRegister(Device, REG_EEPROM);
        if(Res & 0x10)
            Device->HasEEPROM = true;
    }
    return Res;
}

/**
 * Read data from the E1000's EEPROM, if it has one.
 * TODO: Unify
 * @param Device The device to read
 * @param Address The address we expect the data to be at
 * @return uint32_t 32 bits of data in the given address of the EEPROM. 0 if not.
 */
uint32_t E1000ReadEEPROM(e1000_device_t* Device, uint8_t Address) {
    uint32_t Temp = 0;

    if(Device->HasEEPROM) {
        // Tell the device we want the data at given address.
        E1000WriteCommandRegister(Device, REG_EEPROM, (((uint32_t) Address) << 8) | 1);
        // Spinlock until we get the result we expect
        // TODO: Timeout?
        while(!((Temp = E1000ReadCommandRegister(Device, REG_EEPROM)) & (1 << 4)));
    } else {
        // The E1000, if it does not have an EEPROM, instead stores it in internal ROM.
        // So the same thing applies, but with different bits.
        E1000WriteCommandRegister(Device, REG_EEPROM, (((uint32_t) Address) << 2) | 1);
        while(!((Temp = E1000ReadCommandRegister(Device, REG_EEPROM)) & (1 << 1)));
    }

    return (uint16_t)((Temp >> 16) & 0xFFFF);
}

/**
 * Read the E1000's MAC address into the internal buffer.
 * 
 * @param Device The device to read from.
 * @return true The read finished successfully
 * @return false The MMIO base address is non-zero - this is invalid.
 */
bool E1000ReadMAC(e1000_device_t* Device) {
    if(Device->HasEEPROM) {
        uint32_t Temp;
        Temp = E1000ReadEEPROM(Device, 0);
        Device->MAC[0] = Temp & 0xff;
        Device->MAC[1] = Temp >> 8;
        Temp = E1000ReadEEPROM(Device, 1);
        Device->MAC[2] = Temp & 0xff;
        Device->MAC[3] = Temp >> 8;
        Temp = E1000ReadEEPROM(Device, 2);
        Device->MAC[4] = Temp & 0xff;
        Device->MAC[5] = Temp >> 8;
    } else {
        uint8_t* MACBaseChar = (uint8_t*) (Device->MemoryBase + REG_MAC);
        uint32_t* MACBaseLong = (uint32_t*) (Device->MemoryBase + REG_MAC);

        if(MACBaseLong[0] != 0)
            for(size_t i = 0; i < 6; i++)
                Device->MAC[i] = MACBaseChar[i];
        else
            return false;
        
        return true;
    }

}

/**
 * Prepare the receive buffers, tell the device how to handle incoming packets.
 * 
 * @param Device The device to prepare
 */
void E1000InitRX(e1000_device_t* Device) {
    uint8_t* Ptr;
    struct e1000_receive_packet* Packets;

    Ptr = (uint8_t*) (kmalloc(sizeof(struct e1000_receive_packet) * E1000_NUM_RX_DESC + 16));

    Packets = (struct e1000_receive_packet*) Ptr;

    for(size_t i = 0; i < E1000_NUM_RX_DESC; i++) {
        Device->ReceivePackets[i] = (struct e1000_receive_packet*) ((uint8_t*)Packets + i*16);
        Device->ReceivePackets[i]->Address = (size_t) ((uint8_t*) kmalloc((PAGE_SIZE * 2) + 16));
        Device->ReceivePackets[i]->Status = 0;
    }

    E1000WriteCommandRegister(Device, REG_TXDESCLO, (uint32_t) ((size_t)Ptr >> 32));
    E1000WriteCommandRegister(Device, REG_TXDESCHI, (uint32_t) ((size_t)Ptr & 0xFFFFFFFF));

    E1000WriteCommandRegister(Device, REG_RXDESCLO, (size_t) Ptr);
    E1000WriteCommandRegister(Device, REG_RXDESCHI, 0);

    E1000WriteCommandRegister(Device, REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);

    E1000WriteCommandRegister(Device, REG_RXDESCHEAD, 0);
    E1000WriteCommandRegister(Device, REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);

    Device->CurrentReceivePacket = 0;
    E1000WriteCommandRegister(Device, REG_RCTRL, 
        RCTL_EN         |   // ENable
        RCTL_SBP        |   // Store Bad Packets
        RCTL_UPE        |   // Unicast Promiscuous Enable
        RCTL_MPE        |   // Multicast Promiscuous Enable
        RCTL_LBM_NONE   |   // LoopBack Mode
        RCTL_RDMTS_HALF |   // Receive Descriptor Minimum Threshold Size - throw interrupts when the buffer gets half filled
        RCTL_BAM        |   // Broadcast Accept Mode
        RCTL_SECRC      |   // Strip Ethernet CRC
        RCTL_BSIZE_8192     // 8192 byte long buffer.
    );   
}

/**
 * Prepare the transmit buffers, tell the device how to handle outgoing packets.
 * 
 * @param Device The device to prepare
 */
void E1000InitTX(e1000_device_t* Device) {
    uint8_t* Ptr;
    struct e1000_transmit_packet* Packets;

    Ptr = (uint8_t*) (kmalloc(sizeof(struct e1000_transmit_packet) * E1000_NUM_TX_DESC + 16));
    Packets = (struct e1000_transmit_packet*) Ptr;

    for(int i = 0; i < E1000_NUM_TX_DESC; i++) {
        Device->TransmitPackets[i] = (struct e1000_transmit_packet*) ((uint8_t*) Packets + (i * 16));
        Device->TransmitPackets[i]->Address = 0;
        Device->TransmitPackets[i]->Command = 0;
        Device->TransmitPackets[i]->Status = TSTA_DD;
    }

    E1000WriteCommandRegister(Device, REG_TXDESCHI, (uint32_t) ((size_t)Ptr >> 32));
    E1000WriteCommandRegister(Device, REG_TXDESCLO, (uint32_t) ((size_t)Ptr & 0xFFFFFFFF));
    
    E1000WriteCommandRegister(Device, REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);

    E1000WriteCommandRegister(Device, REG_TXDESCHEAD, 0);
    E1000WriteCommandRegister(Device, REG_TXDESCTAIL, 0);

    Device->CurrentTransmitPacket = 0;
   
    E1000WriteCommandRegister(Device, REG_TCTRL, 
        TCTL_EN                     |  // ENable
        TCTL_PSP                    |  // Pad Short Packets
        (15 << TCTL_CT_SHIFT)       |  // Collision Threshold - Attempt to re-send the packet 15 times.
        (0x3F << TCTL_COLD_SHIFT)   |  // Collision Distance
        (0x3 << TCTL_RRTHRES_SHIFT)    // Read Request Threshold - infinity.
    );

    E1000WriteCommandRegister(Device, REG_TIPG, 
        0x0060200A);
}

/**
 * Tell the device that it may send interrupts to inform us of what's going on
 * 
 * @param Device The device to notify
 */
void E1000InitInt(e1000_device_t* Device) {
    E1000WriteCommandRegister(Device, REG_IMASK, 0x1F6DC);
    E1000WriteCommandRegister(Device, REG_IMASK, 0xFF & ~4);
    E1000ReadCommandRegister(Device, 0xC0);
}

/**
 * Handle interrupts fired by the E1000.
 * It will either:
 *  - Help initialise the device
 *  - Handle an incoming packet
 * 
 * @param InterruptContext The interrupt metadata.
 */
void E1000InterruptFired(INTERRUPT_FRAME* InterruptContext) {
    e1000_device_t* NIC = E1000NIC; // TODO: Find device from interrupt frame?
    E1000WriteCommandRegister(NIC, REG_IMASK, 1);
    uint32_t NICStatus = E1000ReadCommandRegister(NIC, 0xC0);

    if(NICStatus & 0x4)
        E1000Uplink(NIC);
    else if(NICStatus & 0x80)
        E1000Receive(NIC);
}

/**
 * Initialise the state of the device, prepare it for further initialization steps.
 * 
 * @param Device The device to initialise
 * @param PCIHeader The device's PCI headers.
 */
void E1000Init(e1000_device_t* Device, pci_device_t* PCIHeader) {
    pci_bar_t BAR = PCIHeader->BARs[0];

    InstallIRQ(11, E1000InterruptFired);

    if(BAR.MMIO) {
        Device->BARType = 0;
        Device->MemoryBase = DecodeVirtualAddress(&KernelAddressSpace, BAR.Address);
        SerialPrintf("[E1000] Device is memory mapped - 0x%p x 0x%d\r\n", Device->MemoryBase, BAR.Length);
        // UpdatePaging
    } else {
        Device->BARType = 1;
        Device->IOBase = BAR.Port;
        SerialPrintf("[E1000] Device is port mapped - 0x%p\r\n", Device->IOBase);
    }

    if(E1000DetectEEPROM(Device))
        SerialPrintf("[E1000] Device has EEPROM\r\n");
    
    E1000ReadMAC(Device);

    SerialPrintf("[E1000] Device's MAC is %d:%d:%d:%d:%d:%d\r\n", Device->MAC[0], Device->MAC[1], Device->MAC[2], Device->MAC[3], Device->MAC[4], Device->MAC[5]);

    // Setup multicast
    for(size_t i = 0; i < 0x80; i++)
        WritePort(0x5200 + i * 4, 0, 4);
    
    E1000InitRX(Device);
    E1000InitTX(Device);
    E1000InitInt(Device);

    SerialPrintf("[E1000] Device ready.\r\n");
}

/**
 * Tell the device that it may connect itself to any networks it finds itself on.
 * 
 * @param Device The device to notify
 */
void E1000Uplink(e1000_device_t* Device) {
    uint32_t Flags = E1000ReadCommandRegister(Device, 0);
    E1000WriteCommandRegister(Device, 0, Flags | 0x40);
}

/**
 * Handle a received packet, placing it in the buffer for other apps to consume.
 * 
 * @param Device The device which received this packet.
 */
void E1000Receive(e1000_device_t* Device) {
    uint16_t Temp;

    while(Device->ReceivePackets[Device->CurrentReceivePacket]->Status & 0x1) {
        Device->ReceivePackets[Device->CurrentReceivePacket]->Status = 0;
        Temp = Device->CurrentReceivePacket;
        Device->CurrentReceivePacket = (Device->CurrentReceivePacket + 1) % E1000_NUM_RX_DESC;

        E1000WriteCommandRegister(Device, REG_RXDESCTAIL, Temp);
    }
}

/**
 * Retrieve the E1000's MAC address.
 * Only valid after E1000ReadMAC is called.
 * 
 * @param Device The device to read
 * @return uint8_t* A pointer to the MAC data.
 */
uint8_t* E1000GetMAC(e1000_device_t* Device) {
    return (uint8_t*) Device->MAC;
}

/**
 * Send a packet of specified length, via the given Device.
 * 
 * @param Device The NIC to send the packet from
 * @param Data The data to send
 * @param Length The length of the data
 * @return int 0 if successful. No other returns.
 */
int E1000Send(e1000_device_t* Device, const void* Data, uint16_t Length) {
    struct e1000_transmit_packet* Target = Device->TransmitPackets[Device->CurrentTransmitPacket];

    Target->Address = (size_t) Data;
    Target->Length = Length;
    Target->Command = CMD_EOP | CMD_IFCS | CMD_RS;
    Target->Status = 0;

    uint8_t temp = Device->CurrentTransmitPacket;
    Device->CurrentTransmitPacket = (Device->CurrentTransmitPacket + 1) % E1000_NUM_TX_DESC;
    E1000WriteCommandRegister(Device, REG_TXDESCTAIL, Device->CurrentTransmitPacket);

    while(!(Device->TransmitPackets[temp]->Status & 0xFF));

    return 0;
}

#ifdef  __cplusplus
}
#endif