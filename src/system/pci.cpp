#include <kernel/chroma.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

/* This file contains functions for accessing the PCI bus,
 *   and devices contained wherein.
 *
 * It allows you to query the bus, as well as communicate with individual devices.
 *
 */

pci_device_t** pci_root_devices = NULL;
pci_entry_t* pci_map = NULL;

//static uint32_t PCIReadConfig(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);

//static const char* PCIGetDeviceName_Subclass(uint8_t DeviceClass, uint8_t Subclass, uint8_t ProgrammableInterface);

//static const char* PCIGetClassName(uint8_t DeviceClass);

void PCIEnumerate() {

    uint8_t bus = 0, device = 0, function = 0;

    uint32_t registerData;

    uint16_t DeviceID, VendorID;

    uint8_t ClassCode, subclass_code;

    SerialPrintf("[  PCI] Started PCI Enumeration.");

    SerialPrintf("\n[  PCI] PCI Scan result:\n[  PCI]");
    do {
        for (device = 0; device <= 31; device++) {
            for (function = 0; function <= 7; function++) {
                registerData = PCIReadConfig(bus, device, 0,
                                             0xC);                      // Read BIST/Header/Latency/Cache Line register
                uint8_t header = (uint8_t) ((registerData & 0x00FF0000)
                        >> 24);         // Header is lower byte of upper word, so mask it off and shift it down
                uint8_t multifunction_bit = header &
                                            0x80;                              // The multifunction bit is the highest bit of the header

                registerData = PCIReadConfig(bus, device, function,
                                             0);                 // Read the Vendor/Device ID register
                VendorID = (uint16_t) (registerData & 0x0000FFFF);                     // Vendor ID is bottom word
                DeviceID = (uint16_t) (registerData >> 16);                            // Device ID is top word

                registerData = PCIReadConfig(bus, device, function, 8);                 // Read the Device Info register
                ClassCode = (uint16_t) (registerData
                        >> 24);                           // Device class is top byte, so shift them down
                subclass_code = (uint16_t) ((registerData >> 16) &
                                            0x00FF);             // Device subclass is same as header - lower byte of higher word. Shift down and mask just like before.
                uint8_t device_progif = (uint16_t) ((registerData & 0x0000FF00)
                        >> 8);  // Device Programming Interface is higher byte of lower word, so mask and shift
                uint8_t device_revision = (uint16_t) (registerData &
                                                      0x000000FF);       // Device revision is lower byte of whole double word. Just mask it.


                /* 0xFFFF is not a valid Vendor ID. This serves as a sanity check.
                 * If this check is true, then nothing is logged and we continue for the next loop.
                 */
                if (VendorID != 0xFFFF) {
                    SerialPrintf("[  PCI]\n[  PCI]\t%x:%x:\n[  PCI]\t\tVendor: %x\n[  PCI]\t\tDevice: %x", bus, device,
                                 VendorID, DeviceID);
                    SerialPrintf("\n[  PCI]\t\tClass: %s\n[  PCI]\t\tDevice Type: %s\n[  PCI]\t\tRevision: %d\n",
                                 PCIGetClassName(ClassCode),
                                 PCIGetDeviceName_Subclass(ClassCode, subclass_code, device_progif), device_revision);
                }

                /* If the PCI Device header tells us that this is not a multifunction device,
                 * and we've already scanned function 0, then there is nothing else to see.
                 * Therefore, stop this loop and move onto device++
                 */
                if (multifunction_bit == 0)
                    break;

            }

        }
    } while (++bus);


}


uint32_t PCIReadConfig(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    uint32_t address;
    uint32_t busLong = (uint32_t) bus;
    uint32_t slotLong = (uint32_t) slot;
    uint32_t functionLong = (uint32_t) function;

    /* ---------------------------------------------------------------
     * | 31 | 30 ... 24 | 23 ... 16 | 15 ... 11 | 10 ... 8 | 7 ... 0 |
     * ---------------------------------------------------------------
     * | EN |  RESERVED |  BUS NUM  | DEVICE NO | FUNC NO  | REG OFF |
     * ---------------------------------------------------------------
     * 
     * NOTE: REG OFF *must* have 0 last two bits (ie. & 0xFC)
     */

    address = (uint32_t) ((busLong << 16) | (slotLong << 11) |
                          (functionLong << 8) | (offset & 0xFC) | ((uint32_t) 0x80000000));

    WritePort(0xCF8, address, 4);


    return ReadPort(0xCFC, 4);
}

const char* PCIGetDeviceName_Subclass(uint8_t DeviceClass, uint8_t Subclass, uint8_t ProgrammableInterface) {
    switch (DeviceClass) {

        case 0x00: {
            switch (Subclass) {
                case 0x00:
                    return "Non-VGA-Compatible device";
                case 0x01:
                    return "VGA-Compatible device";
                default:
                    return "Unknown Unclassified";
            }
        }

        case 0x01: {
            switch (Subclass) {
                case 0x00:
                    return "SCSI Bus Controller";
                case 0x01: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "ISA Compatibility Mode-only IDE Controller";
                        case 0x05:
                            return "PCI Native Mode-only IDE Controller";
                        case 0x0A:
                            return "ISA Compatibility Mode IDE Controller (supports PCI Native Mode)";
                        case 0x0F:
                            return "PCI Native Mode IDE Controller (supports ISA Compatibility Mode)";
                        case 0x80:
                            return "ISA Compatibilty Mode-only IDE Controller (supports Bus Mastering)";
                        case 0x85:
                            return "PCI Native Mode-only IDE Controller (supports Bus Mastering)";
                        case 0x8A:
                            return "ISA Compatibility Mode IDE Controller (supports PCI Native Mode & Bus Mastering)";
                        case 0x8F:
                            return "PCI Native Mode IDE Controller (supports ISA Compatibiliy Mode & Bus Mastering)";
                        default:
                            return "IDE Controller";
                    }
                }
                case 0x02:
                    return "Floppy Disk Controller";
                case 0x03:
                    return "IPI Bus Controller";
                case 0x04:
                    return "RAID Controller";
                case 0x05: {
                    switch (ProgrammableInterface) {
                        case 0x20:
                            return "Single-DMA ATA Controller";
                        case 0x30:
                            return "Chained-DMA ATA Controller";
                        default:
                            return "ATA Controller";
                    }
                }


                case 0x06: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Vendor-Specific Interface SATA Controller";
                        case 0x01:
                            return "AHCI 1.0 SATA Controller";
                        case 0x02:
                            return "Serial Storage Bus SATA Controller";
                        default:
                            return "Serial ATA Controller";
                    }
                }
                case 0x07:
                    return "Serial Attached SCSI (SAS)";

                case 0x08: {
                    switch (ProgrammableInterface) {
                        case 0x01:
                            return "NVMHCI Memory Controller";
                        case 0x02:
                            return "NVMe Memory Controller";
                        default:
                            return "Non-Volatile Memory Controller";

                    }
                }

                case 0x80:
                    return "Other";
                default:
                    return "Unknown Mass Storage Controller";
            }
        }

        case 0x02: {
            switch (Subclass) {
                case 0x00:
                    return "Ethernet Controller";
                case 0x01:
                    return "Token Ring Controller";
                case 0x02:
                    return "FDDI Controller";
                case 0x03:
                    return "ATM Controller";
                case 0x04:
                    return "ISDN Controller";
                case 0x05:
                    return "WorldFip Controller";
                case 0x06:
                    return "PICMG 2.14 Multi Computing";
                case 0x07:
                    return "Infiniband Controller";
                case 0x08:
                    return "Fabric Controller";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Network Controller";
            }
        }

        case 0x03: {
            switch (Subclass) {
                case 0x00: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "VGA Controller";
                        case 0x01:
                            return "8514 VGA Controller";
                        default:
                            return "VGA Compatible Controller";
                    }
                }

                case 0x01:
                    return "XGA Controller";
                case 0x02:
                    return "3D Controller (Not VGA-Compatible)";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Display Controller";
            }
        }

        case 0x04: {
            switch (Subclass) {
                case 0x00:
                    return "Multimedia Video Controller";
                case 0x01:
                    return "Multimedia Audio Controller";
                case 0x02:
                    return "Computer Telephony Device";
                case 0x03:
                    return "Audio Device";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Multimedia Controller";
            }
        }

        case 0x05: {
            switch (Subclass) {
                case 0x00:
                    return "RAM Controller";
                case 0x01:
                    return "Flash Controller";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Memory Controller";
            }
        }

        case 0x06: {
            switch (Subclass) {
                case 0x00:
                    return "Host Bridge";
                case 0x01:
                    return "ISA Bridge";
                case 0x02:
                    return "EISA Bridge";
                case 0x03:
                    return "MCA Bridge";
                case 0x04:
                case 0x09:
                    return "PCI-to-PCI Bridge";
                case 0x05:
                    return "PCMCIA Bridge";
                case 0x06:
                    return "NuBus Bridge";
                case 0x07:
                    return "CardBus Bridge";
                case 0x08:
                    return "RACEway Bridge";
                case 0x0A:
                    return "InfiniBand-to-PCI Host Bridge";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Bridge Device";
            }
        }

        case 0x07: {
            switch (Subclass) {

                case 0x00: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Serial Controller <8250>";
                        case 0x01:
                            return "Serial controller <16450>";
                        case 0x02:
                            return "Serial controller <16550>";
                        case 0x03:
                            return "Serial controller <16650>";
                        case 0x04:
                            return "Serial controller <16750>";
                        case 0x05:
                            return "Serial controller <16850>";
                        case 0x06:
                            return "Serial controller <16950>";
                        default:
                            return "Serial Controller";

                    }
                }

                case 0x01: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Standard Parallel Port";
                        case 0x01:
                            return "Bi-Directional Parallel Port";
                        case 0x02:
                            return "ECP 1.X Compliant Parallel Port";
                        case 0x03:
                            return "IEEE 1284 Parallel Controller";
                        case 0x04:
                            return "IEE 1284 Parallel Target";
                        default:
                            return "Parallel Controller";
                    }
                }
                case 0x02:
                    return "Multiport Serial Controller";

                case 0x03: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Generic Modem";
                        case 0x01:
                            return "Hayes 16450 Compatible Modem";
                        case 0x02:
                            return "Hayes 16550 Compatible Modem";
                        case 0x03:
                            return "Hayes 16650 Compatible Modem";
                        case 0x04:
                            return "Hayes 16750 Compatible Modem";
                        default:
                            return "Modem";
                    }
                }

                case 0x04:
                    return "IEEE 488.1/2 (GPIB) Controller";
                case 0x05:
                    return "Smart Card";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Simple Comms Controller";
            }
        }

        case 0x08: {
            switch (Subclass) {
                case 0x00: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Generic 8259-Compatible PIC";
                        case 0x01:
                            return "ISA-Compatible PIC";
                        case 0x02:
                            return "EISA-Compatible PIC";
                        case 0x03:
                            return "I/O APIC Interrupt Controller";
                        case 0x04:
                            return "I/O(x) APIC Interrupt Controller";
                        default:
                            return "PIC";
                    }
                }

                case 0x01: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Generic 8237-Compatible DMA Controller";
                        case 0x01:
                            return "ISA-Compatible DMA Controller";
                        case 0x02:
                            return "EISA-Compatible DMA Controller";
                        default:
                            return "DMA Controller";
                    }
                }

                case 0x02: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Generic 8254-Compatible Timer";
                        case 0x01:
                            return "ISA-Compatible Timer";
                        case 0x02:
                            return "EISA-Compatible Timer";
                        case 0x03:
                            return "HPET Timer";
                        default:
                            return "Timer";
                    }
                }

                case 0x03: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Generic RTC Controller";
                        case 0x01:
                            return "ISA-Compatible RTC Controller";
                        default:
                            return "RTC Controller";
                    }
                }

                case 0x04:
                    return "PCI Hot-Plug Controller";
                case 0x05:
                    return "SD Host Controller";
                case 0x06:
                    return "IOMMU";
                case 0x80:
                    return "Other";

                default:
                    return "Unknown Base System Peripheral";
            }
        }

        case 0x09: {
            switch (Subclass) {
                case 0x00:
                    return "Keyboard Controller";
                case 0x01:
                    return "Digitiser Pen";
                case 0x02:
                    return "Mouse Controller";
                case 0x03:
                    return "Scanner Controller";

                case 0x04: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Generic Gameport Controller";
                        case 0x10:
                            return "Extended Gameport Controller";
                        default:
                            return "Gameport Controller";
                    }
                }

                case 0x80:
                    return "Other";

                default:
                    return "Unknown Input Device Controller";
            }
        }

        case 0x0A: {
            switch (Subclass) {
                case 0x00:
                    return "Generic Docking Station";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Docking Station";
            }
        }

        case 0x0B: {
            switch (Subclass) {
                case 0x00:
                    return "386 Processor";
                case 0x01:
                    return "486 Processor";
                case 0x02:
                    return "Pentium Processor";
                case 0x03:
                    return "Pentium Pro Processor";
                case 0x10:
                    return "Alpha Processor";
                case 0x20:
                    return "PowerPC Processor";
                case 0x30:
                    return "MIPS Processor";
                case 0x40:
                    return "Co-Processor";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Processor";
            }
        }

        case 0x0C: {
            switch (Subclass) {
                case 0x00: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "Generic Firewire Controller";
                        case 0x10:
                            return "OHCI Firewire Controller";
                        default:
                            return "FireWire (IEEE 1394) Controller";
                    }
                }

                case 0x01:
                    return "ACCESS Bus";
                case 0x02:
                    return "SSA";

                case 0x03: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "UHCI USB Controller";
                        case 0x10:
                            return "OHCI USB Controller";
                        case 0x20:
                            return "EHCI USB (2.0) Controller";
                        case 0x30:
                            return "XHCI USB (3.0) Controller";
                        case 0x80:
                            return "Unspecified USB Controller";
                        case 0xFE:
                            return "USB Device (NOT a Controller)";
                        default:
                            return "USB Controller";
                    }
                }

                case 0x04:
                    return "Fibre Channel";
                case 0x05:
                    return "SMBus";
                case 0x06:
                    return "InfiniBand";

                case 0x07: {
                    switch (ProgrammableInterface) {
                        case 0x00:
                            return "SMIC IPMI Interface";
                        case 0x01:
                            return "Keyboard Controller-Style IPMI Interface";
                        case 0x02:
                            return "Block Transfer IPMI Interface";
                        default:
                            return "IPMI Interface";
                    }
                }

                case 0x08:
                    return "SERCOS Interface (IEC 61491)";
                case 0x09:
                    return "CANbus";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Serial Bus Controller";
            }
        }

        case 0x0D: {
            switch (Subclass) {
                case 0x00:
                    return "IRDA Compatible Controller";
                case 0x01:
                    return "Consumer IR Controller";
                case 0x10:
                    return "RF Controller";
                case 0x11:
                    return "Bluetooth Controller";
                case 0x12:
                    return "Broadband Controller";
                case 0x20:
                    return "Ethernet Controller (802.1a)";
                case 0x21:
                    return "Ethernet Controller (802.1b)";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Wireless Controller";
            }
        }

        case 0x0E: {
            switch (Subclass) {
                case 0x00:
                    return "I20";
                default:
                    return "Unknown Intelligent Controller";
            }
        }

        case 0x0F: {
            switch (Subclass) {
                case 0x01:
                    return "Satellite TV Controller";
                case 0x02:
                    return "Satellite Audio Controller";
                case 0x03:
                    return "Satellite Voice Controller";
                case 0x04:
                    return "Satellite Data Controller";
                default:
                    return "Unknown Satelllite Comms Controller";
            }
        }

        case 0x10: {
            switch (Subclass) {
                case 0x00:
                    return "Network & Computing Codec";
                case 0x10:
                    return "Entertainment Codec";
                case 0x80:
                    return "Other Codec";
                default:
                    return "Unknown Encryption Controller";
            }
        }

        case 0x11: {
            switch (Subclass) {
                case 0x00:
                    return "DPIO Modules";
                case 0x01:
                    return "Performance Counters";
                case 0x10:
                    return "Communication Synchronizer";
                case 0x20:
                    return "Signal Processing Management";
                case 0x80:
                    return "Other";
                default:
                    return "Unknown Signal Processing Controller";
            }
        }

        case 0x12:
            return "Processing Accelerator";

        case 0x13:
            return "Non-Essential Instrumentation";

        case 0x14:
        case 0x41:
            return "Reserved";

        case 0x40:
            return "Co-Processor";

        case 0xFF:
            return "Unassigned Class";

    }

    return "Invalid Device!";
}

const char* PCIGetClassName(uint8_t DeviceClass) {

    switch (DeviceClass) {
        case 0x00:
            return "Unclassified";
        case 0x01:
            return "Mass Storage Controller";
        case 0x02:
            return "Network Controller";
        case 0x03:
            return "Display Controller";
        case 0x04:
            return "Multimedia Controller";
        case 0x05:
            return "Memory Controller";
        case 0x06:
            return "Bridge Device";
        case 0x07:
            return "Simple Communication Controller";
        case 0x08:
            return "Base System Peripheral";
        case 0x09:
            return "Input Device Controller";
        case 0x0A:
            return "Docking Station";
        case 0x0B:
            return "Processor";
        case 0x0C:
            return "Serial Bus Controller";
        case 0x0D:
            return "Wireless Controller";
        case 0x0E:
            return "Intelligent Controller";
        case 0x0F:
            return "Satellite Communication Controller";
        case 0x10:
            return "Encryption Controller";
        case 0x11:
            return "Signal Processing Controller";
        case 0x12:
            return "Processing Accelerator";
        case 0x13:
            return "Non-Essential Instrumentation";
        case 0x14:
            return "Reserved";
        case 0x40:
            return "Co-Processor";
        case 0x41:
            return "Reserved";
        case 0xFF:
            return "Unassigned Class";
        default:
            return "Unknown Category";
    }

    return "Invalid device!";
}

#ifdef  __cplusplus
}
#endif