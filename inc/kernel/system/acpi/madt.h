#pragma once
#include <stdint.h>
#include <kernel/system/acpi/rsdt.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

namespace ACPI {

    // Multiple APIC Descriptor Table. Enumerates every available APIC.
    class MADT {
    private:
        void* Address = 0;

    public:

        // The type of a table entry.
        enum Type {
            LAPIC = 0,
            IOAPIC = 1,
            ISO = 2,
            NMI = 4,
            LAPIC_OVERRIDE = 5
        };

        // The header to a MADT Table Entry.
        struct RecordTableEntry {
            uint8_t Type;   // The Type Enum value that represents this entry.
            uint8_t Length; // The length in bytes of this entry.
        } __attribute__((packed));

        // The data of a Local APIC table Entry.
        struct LAPICEntry {
            RecordTableEntry Header;
            uint8_t Core;   // The Core ID where the Local APIC is stored
            uint8_t APIC;   // The ID of the APIC for the above Core ID.
            uint32_t Flags; // Capability flags for the APIC.
        } __attribute__((packed));

        // The data of an IO (global) APIC table entry.
        struct IOAPICEntry {
            RecordTableEntry Header;
            uint8_t APIC;     // The ID of the global APIC.
            uint8_t Unused;    
            uint32_t Address; // The Base address of the MMIO port of the APIC.
            uint32_t GSI;     // Global System Interrupt number.
        } __attribute__((packed));

        // The data of an Interrupt Source Override entry - allows interrupt redirection.
        struct ISOEntry {
            RecordTableEntry Header;
            uint8_t Bus; 
            uint8_t IRQ;
            uint32_t Interrupt;
            uint16_t Flags;
        } __attribute__((packed));

        // The data of a Non-Maskable Interrupt source entry.
        struct NMIEntry {
            RecordTableEntry Header;
            uint8_t Core;
            uint16_t Flags;
            uint8_t LocalInterrupt;
        } __attribute__((packed));

        // The data of a Local APIC Override Entry.
        struct LocalOverrideEntry {
            RecordTableEntry Header;
            uint8_t Reserved;
            size_t Address; // The 64 bit address of the Local APIC.
        } __attribute__((packed));

        struct MADTHeader {
            ACPIHeader Header;
            uint32_t LocalAPIC;
            uint32_t Flags;
            RecordTableEntry Table[];
        } __attribute__((packed));

        /***************************************
         ***************************************
         ***************************************/

        static MADT* instance;
        MADTHeader* Header = 0;
        size_t LocalAPICBase = 0;

        MADT();

        void LogDump();
        void Initialize();
        // Get the byte of the end of the table.
        size_t GetEndOfTable();
        // Get all of the entries in the table, as an array.
        RecordTableEntry* GetTableEntries();
        // Get an array of pointers to IOAPIC entries. Should only be one.
        IOAPICEntry** GetIOApicEntries();
        // Get an array of pointers to ISO entries.
        ISOEntry** GetISOEntries();

    };
}