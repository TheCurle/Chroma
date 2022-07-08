#include <kernel/system/acpi/madt.h>
#include <kernel/chroma.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

using namespace ACPI;

ACPI::MADT* MADT::instance;

MADT::MADT() {
    instance = this;
}

size_t MADT::GetEndOfTable() {
    return ((size_t) &Header->Header) + Header->Header.Length;
}

void MADT::Init() {
    SerialPrintf("[ ACPI] Loading Multiple APIC Descriptor Tables..\r\n");
    Address = ACPI::RSDP::instance->FindEntry("APIC");
    Header = reinterpret_cast<MADTHeader*>(Address);

    LocalAPICBase = Header->LocalAPIC;

    // TODO: Check whether the Base is identity mapped
}

MADT::IOAPICEntry** MADT::GetIOApicEntries() {
    MADT::RecordTableEntry* table;
    table = Header->Table;
    auto** entries = (MADT::IOAPICEntry**) kmalloc(255);

    size_t count = 0;

    while ((size_t) table < GetEndOfTable()) {
        table = (MADT::RecordTableEntry*) (((size_t) table) + table->Length);

        if (table->Type == MADT::Type::IOAPIC) {
            MADT::IOAPICEntry* io_apic = reinterpret_cast<MADT::IOAPICEntry*>(table);
            entries[count] = (MADT::IOAPICEntry*) ((size_t) io_apic);
            count++;
        }
    }

    entries[count] = 0;
    return entries;
}

MADT::ISOEntry** MADT::GetISOEntries() {
    MADT::RecordTableEntry* table = Header->Table;
    auto** entries = (MADT::ISOEntry**) kmalloc(255);

    size_t count = 0;

    while ((size_t) table < GetEndOfTable()) {
        table = (MADT::RecordTableEntry*) (((size_t) table) + table->Length);

        if (table->Type == MADT::Type::ISO) {
            MADT::ISOEntry* io_apic = reinterpret_cast<MADT::ISOEntry*>(table);
            entries[count] = (MADT::ISOEntry*) ((size_t) io_apic);
            count++;
        }
    }

    entries[count] = 0;
    return entries;
}

MADT::RecordTableEntry* MADT::GetTableEntries() {
    return Header->Table;
}