#include <driver/io/apic.h>
#include <kernel/system/acpi/madt.h>
#include <kernel/system/io.h>
#include <kernel/system/memory.h>
#include "kernel/system/core.hpp"
#include "kernel/chroma.h"

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

//TODO: remove
#define UNUSED(x) (void)x

// All of these functions are in the Device namespace.
using namespace Device;

Device::APIC* APIC::driver;

APIC::APIC() {
    Ready = false;
    driver = this;
}

void APIC::WriteIO(size_t Base, uint32_t Register, uint32_t Data) {
    // MMIO - Set the base to the register we want to write into, 
    // 16 bytes higher the data we want to write, and the hardware does the rest.
    *((volatile uint32_t*) Base) = Register;
    *((volatile uint32_t*) (Base + 16)) = Data;
}

uint32_t APIC::ReadIO(size_t Base, uint32_t Register) {
    // MMIO- Set the base to the register we want to read.
    // The data we want will be put 16 bytes higher.

    *((volatile uint32_t*) Base) = Register;

    return *((volatile uint32_t*) (Base + 16));
}

uint32_t APIC::ReadRegister(uint32_t Register) {
    return *((volatile uint32_t*) ((size_t*) Address + Register));
}

void APIC::WriteRegister(uint32_t Register, uint32_t Data) {
    *((volatile uint32_t*) ((size_t*) Address + Register)) = Data;
}

void APIC::Enable() {
    // Set the correct bits in the SIVR register. The Local APIC will be enabled.
    WriteRegister(Registers::SIVR, ReadRegister(Registers::SIVR) | 0x100);
}

void APIC::SendEOI() {
    SerialPrintf("[  IRQ] EOI\r\n");
    WriteRegister(Registers::EOI, 0);
}

bool APIC::IsReady() const {
    return Ready;
}

void APIC::Init() {
    Device::RegisterDevice(this);

    SerialPrintf("[ ACPI] Enabling APICs...\r\n");

    SerialPrintf("[ ACPI] Memory Mapping APICs..\r\n");
    for (int i = 0; i < 3; i++) {
        MapVirtualPage(&KernelAddressSpace, (size_t) Address + (i * PAGE_SIZE), (size_t) Address + i * PAGE_SIZE, 3);
    }

    Address = (void*) ACPI::MADT::instance->LocalAPICBase;
    SerialPrintf("[ MADT] The APIC of this core is at 0x%p\r\n", (size_t) Address);

    if (Address == nullptr) {
        SerialPrintf("[ ACPI] Unable to locate APICs.\r\n");
        for (;;) { }
    }

    // Remap PICs

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */

    WritePort(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4, 1);  // starts the initialization sequence (in cascade mode)
    WritePort(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4, 1);
    WritePort(PIC1_DATA, 0xF0, 1);                 // ICW2: Master PIC vector offset
    WritePort(PIC2_DATA, 0xF0, 1);                 // ICW2: Slave PIC vector offset
    WritePort(PIC1_DATA, 4, 1);                       // ICW3: tell Master PIC t
    WritePort(PIC2_DATA, 2, 1);                       // ICW3: tell Slave PIC it

    WritePort(PIC1_DATA, ICW4_8086, 1);               // ICW4: have the PICs use 8086 mode (and not
    WritePort(PIC2_DATA, ICW4_8086, 1);

    // Disable PIC1 and 2
    WritePort(0xA1, 0xFF, 1);
    WritePort(0x21, 0xFF, 1);
    // Write "Local APIC Enabled" to the APIC Control Register.
    WriteModelSpecificRegister(0x1B, (ReadModelSpecificRegister(0x1B) | 0x800) & ~(1 << 10));
    Enable();

    IOAPICs = ACPI::MADT::instance->GetIOApicEntries();
    ISOs = ACPI::MADT::instance->GetISOEntries();

    SerialPrintf("[ APIC] Core %u's LAPIC (ID %u) is at 0x%x.\r\n", GetCurrentCore(), ReadRegister(LAPIC_ID), Address);
    for (int i = 0; IOAPICs[i] != nullptr; i++) {
        SerialPrintf("[ APIC] Info for IOAPIC %u\r\n", i);
        size_t addr = (IOAPICs[i]->Address);
        SerialPrintf("[ APIC] \tAddress: 0x%x\r\n", addr);
        uint32_t raw = ReadIO(addr, 0x1);
        auto* meta = (IOAPICMeta*) &raw;

        SerialPrintf("[ APIC] \tVersion: %u\r\n", meta->Version);
        SerialPrintf("[ APIC] \tMax Vector: %u\r\n", meta->MaxRedirect);
        SerialPrintf("[ APIC] \tGSI Start: %u\r\n", IOAPICs[i]->GSI);
        SerialPrintf("[ APIC] \tGSI End: %u\r\n", IOAPICs[i]->GSI + meta->MaxRedirect);
    }

    for (int i = 0; ISOs[i] != nullptr; i++) {
        SerialPrintf("[ APIC] Info for Interrupt Source Override %u:\r\n", i);
        SerialPrintf("[ APIC] \tSource: %u\r\n", ISOs[i]->IRQ);
        SerialPrintf("[ APIC] \tTarget: %u\r\n", ISOs[i]->Interrupt);

        SerialPrintf("[ APIC] \tActive %s.\r\n", ISOs[i]->Flags & 0x4 ? "high" : "low");
        SerialPrintf("[ APIC] \t%s triggered.\r\n", ISOs[i]->Flags & 0x100 ? "Edge" : "Level");
    }

    Ready = true;

    SerialPrintf("[ APIC] Testing LAPIC.. Sending an IRQ 5.\r\n");
    WriteRegister(ICR2, ReadRegister(LAPIC_ID) << 24);
    WriteRegister(ICR1, 5);
}

uint32_t APIC::GetMaxRedirect(uint32_t ID) {
    size_t address = (IOAPICs[ID]->Address);
    uint32_t raw = (ReadIO(address, 0x1));

    auto* table = (IOAPICMeta *) & raw;
    return table->MaxRedirect;
}

int APIC::GetCurrentCore() {
    return ReadRegister(Registers::LAPIC_ID) >> 24;
}

void APIC::PreinitializeCore(size_t Core) {
    WriteRegister(Registers::ICR2, Core << 24);
    WriteRegister(Registers::ICR1, 0x500);
}

void APIC::InitializeCore(size_t Core, size_t EntryPoint) {
    WriteRegister(Registers::ICR2, Core << 24);
    WriteRegister(Registers::ICR1, 0x600 | ((uint32_t) (EntryPoint / PAGE_SIZE)));
}

void APIC::SetInternal(uint8_t Vector, uint32_t irq, uint16_t Flags, size_t CoreID, int Status) {

    size_t temp = Vector;
    int64_t target = -1;

    for (size_t i = 0; IOAPICs[i] != nullptr; i++)
        if (IOAPICs[i]->GSI <= irq)
            if (IOAPICs[i]->GSI + GetMaxRedirect(i) > irq) {
                target = i;
                break;
            }

    if (target == -1) {
        SerialPrintf("[ APIC] No ISO found when setting up redirect.\r\n");
        return;
    }

    if (Flags & 2)
        temp |= (1 << 13);

    if (Flags & 8)
        temp |= (1 << 15);

    if (!Status)
        temp |= (1 << 16);

    temp |= (1 << 11);

    temp |= (((size_t) Core::GetCore((int32_t) CoreID)->LocalAPIC) << 56);
    uint32_t IORegister = (irq - IOAPICs[target]->GSI) * 2 + 16;

    SerialPrintf("[ APIC] Setting interrupt %u, redirect %u, on LAPIC %u(%u) of core %u, address 0x%p, register 0x%p, data 0x%p\r\n", irq, Vector, Core::GetCore(CoreID)->LocalAPIC, target, CoreID, IOAPICs[target]->Address, IORegister, temp);

    WriteIO(IOAPICs[target]->Address, IORegister, (uint32_t) temp);
    WriteIO(IOAPICs[target]->Address, IORegister + 1, (uint32_t)(temp >> 32));
}

void APIC::Set(size_t CPU, uint8_t IRQ, int Enabled) {
    SerialPrintf("[ APIC] %s IRQ %u for CPU %u.\r\n", Enabled ? "Enabling" : "Disabling", IRQ, CPU);
    for(size_t i = 0; ISOs[i] != nullptr; i++) {
        // We need to make sure we take into account the overrides, so check whether any IRQ is overriden
        if (ISOs[i]->IRQ == IRQ) {
            SerialPrintf("[ APIC] ISO %u matches for IRQ %u, mapping to %u with GSI %u.\r\n", i, IRQ, ISOs[i]->IRQ + 0x20, ISOs[i]->Interrupt);
            SetInternal(ISOs[i]->IRQ + 0x20, ISOs[i]->Interrupt, ISOs[i]->Flags, CPU, Enabled);
            return;
        }
    }

    // If there are no overrides, we can just remap it upwards
    SetInternal(IRQ + 32, IRQ, 0, CPU, Enabled);
}