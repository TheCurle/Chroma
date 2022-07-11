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
    // Set the correct bits in the SIVR register. The APIC will be enabled.
    WriteRegister(Registers::SIVR, ReadRegister(Registers::SIVR) | 0x1FF);
}

void APIC::SendEOI() {
    *((volatile uint32_t*) 0xfee000B0) = 0;
}

bool APIC::IsReady() {
    return Ready;
}

void APIC::Init() {
    Device::RegisterDevice(this);

    SerialPrintf("[ ACPI] Enabling APICs...\r\n");

    for (int i = 0; i < 3; i++) {
        MapVirtualPage(&KernelAddressSpace, (size_t) Address + i * PAGE_SIZE, (size_t) Address + i * PAGE_SIZE, 3);
    }

    Address = (void*) ACPI::MADT::instance->LocalAPICBase;
    SerialPrintf("[ MADT] The APIC of this core is at 0x%p\r\n", (size_t) Address);

    if (Address == nullptr) {
        SerialPrintf("[ ACPI] Unable to locate APICs.\r\n");
        for (;;) { }
    }

    // Write "Local APIC Enabled" to the APIC Control Register.
    WriteModelSpecificRegister(0x1B, (ReadModelSpecificRegister(0x1B) | 0x800) & ~(1 << 10));
    Enable();

    IOAPICs = ACPI::MADT::instance->GetIOApicEntries();
    ISOs = ACPI::MADT::instance->GetISOEntries();

    Ready = true;
}

uint32_t APIC::GetMaxRedirect(uint32_t ID) {
    size_t address = (IOAPICs[ID]->Address);
    uint32_t raw = (ReadIO(address, 0x1));

    IOAPICMeta* table = (IOAPICMeta * ) & raw;
    return table->MaxRedirect;
}

int APIC::GetCurrentCore() {
    return ReadRegister(Registers::LAPIC_ID) >> 24;
}

void APIC::PreinitializeCore(int Core) {
    WriteRegister(Registers::ICR2, Core << 24);
    WriteRegister(Registers::ICR1, 0x500);
}

void APIC::InitializeCore(int Core, size_t EntryPoint) {
    WriteRegister(Registers::ICR2, Core << 24);
    WriteRegister(Registers::ICR1, 0x600 | ((uint32_t) (EntryPoint / PAGE_SIZE)));
}

void APIC::SetInternal(uint8_t Vector, uint32_t GSI, uint16_t Flags, int CoreID, int Status) {

    size_t temp = Vector;
    int64_t target = -1;

    for (size_t i = 0; IOAPICs[i] != 0x0; i++)
        if (IOAPICs[i]->GSI <= GSI)
            if (IOAPICs[i]->GSI + GetMaxRedirect(i) > GSI) {
                target = i;
                continue;
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

    temp |= (((size_t) Core::GetCore(CoreID)->LocalAPIC) << 56);
    uint32_t IORegister = (GSI - IOAPICs[target]->GSI) * 2 + 0x10;

    WriteIO(IOAPICs[target]->Address, IORegister, (uint32_t) temp);
    WriteIO(IOAPICs[target]->Address, IORegister + 1, (uint32_t)(temp >> 32));
}

void APIC::Set(int CPU, uint8_t IRQ, int Enabled) {
    for(size_t i = 0; ISOs[i] != 0; i++) {
        // We need to make sure we take into account the overrides, so check whether any IRQ is overriden
        if (ISOs[i]->IRQ == IRQ) {
            SetInternal(ISOs[i]->IRQ + 0x20, ISOs[i]->Interrupt, ISOs[i]->Flags, CPU, Enabled);
            return;
        }
    }

    // If there are no overrides, we can just remap it upwards
    SetInternal(IRQ + 0x20, IRQ, 0, CPU, Enabled);
}

void APIC::SendInterCoreInterrupt(int Core, uint32_t Interrupt) {
    WriteRegister(ICR2, (Core << 24));
    WriteRegister(ICR1, (1 << 14) | Interrupt);
}