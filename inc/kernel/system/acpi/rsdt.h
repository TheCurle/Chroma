#pragma once
#include <stdint.h>
#include <stddef.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

namespace ACPI {

    // The header of all System Descriptor Tables in the ACPI system.
    struct ACPIHeader {
        char Signature[4];
        uint32_t Length;
        uint8_t Revision;
        uint8_t Checksum;
        char OEMID[6];
        char OEMTableID[8];
        uint32_t OEMRevision;
        uint32_t CreatorID;
        uint32_t CreatorRevision;
    } __attribute__((packed));

    // Root System Description Pointer table container.
    class RSDP {
        public:

        // Header of RSDP Version 1 entries.
        struct DescriptorV1 {
            char Signature[8];
            uint8_t Checksum;
            char OEM[6];
            uint8_t Revision;
            uint32_t RSDT;
        } __attribute__((packed));

        // Header of RSDP Version 2 entries.
        struct DescriptorV2 {
            DescriptorV1 Header;
            uint32_t Length;
            size_t XSDT;
            uint8_t Checksum;
            uint8_t Reserved[3];
        } __attribute__((packed));

        // RSDP Entries themselves.
        struct RSDT {
            ACPIHeader Header;
            uint32_t* OtherSDTs;
        } __attribute__((packed));

        RSDP();
        static RSDP* instance;

        // Find the RSDP pointer in memory.
        void* GetRSDP();
        // Find the Fixed ACPI Descriptor (FADT) in memory.
        void* GetFACP(RSDT* Root);
        // Prepare virtual mapping of the RSDT
        void PagingInit();

        // Prepare the RSDT for reading.
        __attribute__((unused)) void Init(size_t RSDP);

        // Find the table with the specified name.
        void* FindEntry(const char* Name);

        // Dump all available information to the system log.
        void LogDump();

    private:
        size_t Version = 0;
        DescriptorV2* Descriptor;
        RSDT* Table;
    };
}