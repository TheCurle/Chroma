#include <kernel/chroma.h>
#include <kernel/system/acpi/madt.h>
#include <driver/io/apic.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

int Cores = 0;
volatile bool Ready = false;

Core* Core::Processors[Constants::Core::MAX_CORES];
TSS64 Tasks[Constants::Core::MAX_CORES];
ACPI::MADT::LAPICEntry* LAPICs[Constants::Core::MAX_CORES];

void InitPIC() {
    SerialPrintf("[  CPU] Disabling 8259 PICs\n");
    WritePort(0x20, 0x11, 1);
    WritePort(0xA0, 0x11, 1);

    WritePort(0x21, 0xE0, 1);
    WritePort(0xA1, 0xE8, 1);

    WritePort(0x21, 0x4, 1);
    WritePort(0xA1, 0x2, 1);

    WritePort(0x21, 0x1, 1);
    WritePort(0xA1, 0x1, 1);

    WritePort(0x21, 0xFF, 1);
    WritePort(0xA1, 0xFF, 1);

    WritePort(0x22, 0x70, 1);
    WritePort(0x23, 0x01, 1);
}

extern "C" [[noreturn]] void initcpu() {
    // Init APIC
    WriteModelSpecificRegister(0x1B, (ReadModelSpecificRegister(0x1B) | 0x800) & ~(1 << 10));
    Device::APIC::driver->Enable();

    __asm__ __volatile__("mov %%fs, %0" : : "r" (Device::APIC::driver->GetCurrentCore()) : );

    SerialPrintf("[CORE] Core %d ready.\r\n", Device::APIC::driver->GetCurrentCore());
    __asm__ __volatile__("cli");
    Ready = true;
    __asm__ __volatile__("sti");

    for (;;) { }
}

Core::Core(size_t APIC, size_t ID) {
    Device::APIC::driver->PreinitializeCore(APIC);
    GetCore(ID)->LocalAPIC = APIC;

    Bootstrap();
    //SetupData(ID);

    Device::APIC::driver->InitializeCore(APIC, reinterpret_cast<size_t>(initcpu));
    __asm__ __volatile__("mov %%fs, %0" : : "r" (Device::APIC::driver->GetCurrentCore()) : );

    while (!Ready) {
        __asm__ __volatile__ ("nop");
    }

    Ready = false;
}

void Core::PreInit() {
    for (size_t i = 0; i < Constants::Core::MAX_CORES; i++) {
        Processors[i] = new Core();
    }
}

void Core::Init() {

    using namespace ACPI;

    Ready = false;
    SerialPrintf("[ CORE] Enabling Multiprocessing\r\n");

    memset(Tasks, 0, Constants::Core::MAX_CORES * sizeof(TSS64));
    for (size_t i = 0; i < Constants::Core::MAX_CORES; i++)
        LAPICs[i] = nullptr;

    // Parse MADT for cores
    MADT::RecordTableEntry* table = MADT::instance->GetTableEntries();
    Cores = 0;

    // While there are more entries in the table..
    while ((size_t) table < MADT::instance->GetEndOfTable()) {
        // Check for a LAPIC record (which indicates a unique physical core)
        if (table->Type == MADT::Type::LAPIC) {
            // Find the data for the LAPIC with a reinterpret
            auto* lapic = reinterpret_cast<MADT::LAPICEntry*>(table);

            // Set the current ID
            LAPICs[Cores] = lapic;
            // Tell the core that its' LAPIC is the current id
            Core::GetCore(Cores)->LocalAPIC = lapic->APIC;
            // Move to the next core if there is one.
            Cores++;
        }
        // Move to the next entry (by skipping the length of the current entry)
        table = (MADT::RecordTableEntry*) (((size_t) table) + table->Length);
    }

    SerialPrintf("[ CORE] Found %d core(s).\r\n", Cores);
    if (Cores > 1)
        SerialPrintf("[ CORE] Bringing up other cores.\r\n");

    // For each non-bootstrap core, initialize the necessary data and populate the array.
    for (int i = 0; i < Cores; i++) {

        if (Device::APIC::driver->GetCurrentCore() != LAPICs[i]->Core) {
            SerialPrintf("[ CORE] Enabling core %d.\r\n", i);
            delete Processors[i];
            Core* c = new Core(LAPICs[i]->APIC, LAPICs[i]->Core);
            Processors[i] = c;
        }
    }
}

void Core::Bootstrap() {
    // TODO
}

void Core::StackTrace(size_t Cycles) {
    StackFrame* stack;

    __asm__ __volatile__ ("mov %%rbp, %[dest]" : [dest] "=r"(stack) : :);
    SerialPrintf("[Trace] Beginning stack trace. RBP is currently 0x%p\r\n", stack);
    for (size_t frame = 0; stack != nullptr && frame < Cycles; ++frame) {
        SerialPrintf("[Trace] (%d) 0x%p:  0x%p \r\n", frame, stack->rbp, stack->rip);
        stack = stack->rbp;
    }
    SerialPrintf("[Trace] Stack trace over.\r\n");
}
