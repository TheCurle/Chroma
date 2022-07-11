#include <kernel/system/acpi/rsdt.h>
#include <kernel/chroma.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

using namespace ACPI;

ACPI::RSDP* RSDP::instance;

void* RSDP::GetRSDP() {

    // Search physical memory (via the direct region) for the "RSD PTR " magic string.
    // TODO: this is extremely slow!
    for (size_t i = TO_DIRECT(0x80000); i < TO_DIRECT(0x100000); i += 16) {
        if (i == TO_DIRECT(0xA0000)) {
            i = TO_DIRECT(0xE0000 - 16);
            continue;
        }

        if (strcmp((char*) i, "RSD PTR ")) {
            return (void*) i;
        }
    }

    return nullptr;
}

RSDP::RSDP() {
    instance = this;
    Table = (RSDT*) 0;
    Descriptor = (DescriptorV2*) 0;
}

void* RSDP::FindEntry(const char* Name) {
    // Take the pointer to the RSD Table from the Direct region.
    auto* table = reinterpret_cast<RSDP::RSDT*>(TO_DIRECT(Descriptor->Header.RSDT));
    size_t entries = (table->Header.Length - sizeof(table->Header)) / 4;


    // For each entry: if valid, search for the specified name.
    for (size_t i = 0; i < entries; i++) {
        if (table->OtherSDTs[i] == 0x0)
            continue;

        ACPIHeader* header = reinterpret_cast<ACPIHeader*>(table->OtherSDTs[i]);

        if (strcmp(header->Signature, Name))
            return (void*) header;
    }

    return nullptr;
}

void* RSDP::GetFACP(RSDT* Root) {
    UNUSED(Root);
    return FindEntry("FACP");
}

void RSDP::Init() {
    SerialPrintf("[ ACPI] Loading ACPI subsystem..\r\n");

    Descriptor = (DescriptorV2*) GetRSDP();
    Table = (RSDT *) TO_DIRECT(Descriptor->Header.RSDT);
}

void RSDP::PagingInit() {
    // TODO: check whether the identity mapping makes this necessary.
}

