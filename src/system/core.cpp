#include <kernel/chroma.h>
#include <kernel/system/acpi/madt.h>
#include <driver/io/apic.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

extern size_t startCore;
extern size_t endCore;

int Cores = 0;
volatile bool Ready = false;

Core Core::Processors[Constants::Core::MAX_CORES];
TSS64 Tasks[Constants::Core::MAX_CORES];
ACPI::MADT::LAPICEntry* LAPICs[Constants::Core::MAX_CORES];

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
    SetupData(ID);

    Device::APIC::driver->InitializeCore(APIC, reinterpret_cast<size_t>(initcpu));

    while (!Ready) {
        __asm__ ("nop");
    }

    Ready = false;
}

void Core::Init() {
    using namespace ACPI;

    Ready = false;
    SerialPrintf("[CORE] Enabling Multiprocessing\n");

    memset(Tasks, 0, Constants::Core::MAX_CORES * sizeof(TSS64));
    for (size_t i = 0; i < Constants::Core::MAX_CORES; i++)
        LAPICs[i] = nullptr;

    // Parse MADT for cores
    MADT::RecordTableEntry* table = MADT::instance->GetTableEntries();
    Cores = 0;

    // While there are more entries in the table..
    while ((size_t) table < MADT::instance->GetEndOfTable()) {
        // Move to the next entry (by skipping the length of the current entry)
        table = (MADT::RecordTableEntry*) (((size_t) table) + table->Length);
        // Check for a LAPIC record (which indicates a unique physical core)
        if (table->Type == MADT::Type::LAPIC) {
            // Find the data for the LAPIC with a reinterpret
            MADT::LAPICEntry* lapic = reinterpret_cast<MADT::LAPICEntry*>(table);

            // Set the current ID
            LAPICs[Cores] = lapic;
            // Tell the core that its' LAPIC is the current id
            Core::GetCore(Cores)->LocalAPIC = lapic->APIC;
            // Move to the next core if there is one.
            Cores++;
        }
    }

    SerialPrintf("[CORE] Found %d cores.\n", Cores);

    if (Cores > 1)
        SerialPrintf("[CORE] Bringing up other cores.\n");

    // For each non-bootstrap core, initialize the necessary data and populate the array.
    for (int i = 0; i < Cores; i++) {
        SerialPrintf("[CORE] Enabling core %d.\n", i);
        if (Device::APIC::driver->GetCurrentCore() != LAPICs[i]->Core)
            Processors[i] = Core(LAPICs[i]->APIC, LAPICs[i]->Core);
    }
}

void Core::Bootstrap() {
    // TODO
}

void Core::SetupData(size_t Core) {
    // Write page tables
    GetCore(Core)->AddressSpace->PML4 = GetCurrent()->AddressSpace->PML4;
    *((size_t*) Initialization::PAGETABLES) = (size_t) GetCore(Core)->AddressSpace->PML4;

    // Prepare stack
    memset(GetCore(Core)->StackData, 0, Constants::Core::STACK_SIZE);
    *((size_t*) Initialization::STACK) = (size_t) GetCore(Core)->StackData + Constants::Core::STACK_SIZE;

    // GDT = 0x680
    // IDT = 0x690
    __asm__ __volatile__("sgdt (0x680)\n sidt (0x690)");

    // Set startup address
    *((size_t*) Initialization::STARTUP) = (size_t) &initcpu;
}


void Core::StackTrace(size_t Cycles) {
    StackFrame* stack;

    __asm__ __volatile__ ("mov %%rbp, %[dest]" : [dest] "=r"(stack) : :);
    SerialPrintf("[Trace] Beginning stack trace. RBP is currently 0x%p\r\n", stack);
    for (size_t frame = 0; stack != 0 && frame < Cycles; ++frame) {
        SerialPrintf("[Trace] (%d) 0x%p:  0x%p \r\n", frame, stack->rbp, stack->rip);
        stack = stack->rbp;
    }
    SerialPrintf("[Trace] Stack trace over.\r\n");
}
