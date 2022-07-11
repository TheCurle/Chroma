#include <kernel/chroma.h>
#include <kernel/system/process/process.h>
#include "driver/io/apic.h"

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

struct DeadProcessData {
    size_t PID;
    uint32_t Status;
    uint32_t ReturnCode;
};

// Details on dead processes.
lainlib::vector<DeadProcessData> deadProcesses;

// An array of pointers to the header of each active process.
Process** processes;
// An array of pointers to the header of each process active on the current core.
Process* processesPerCore[MAX_CORES];

ProcessManager* ProcessManager::instance;

// A lock "counter". This can be increased and decreased at will.
// Positive values mean the process is being locked by another, zero and negative means it is active.
int locked = 0;

// Counts how many process are slated to be killed.
// These will be cleaned up by a reaper thread in the background.
int dying = 0;

// Used in HandleRequest for load balancing if requested.
size_t lastSelectedCPU = 0;

// Whether the current process is fully loaded into memory and ready to run.
bool loaded = false;

// A ticketlock that prevents two threads being created at the same time.
ticketlock_t creatorlock;
// A ticketlock that prevents two threads from being switched into at the same time.
ticketlock_t switcherlock;

// The PID of the next process to be created. The kernel is always 0.
size_t nextPID = 1;

// A counter used in scheduling.
size_t lastProcess = 0;

Process* Process::Current() {
    return processesPerCore[Device::APIC::driver->GetCurrentCore()];
}

void Process::SetCurrent(Process* Target) {
    processesPerCore[Device::APIC::driver->GetCurrentCore()] = Target;
}

void lockProcess() {
    locked++;
}

void unlockProcess() {
    locked--;
}

bool isLocked() {
    return locked != 0;
}

[[noreturn]] void NullProcess() {
    __asm__("cli");
    for(;;) {
        ProcessManager::yield();
    }
}

void Process::Destroy() {
    SetActive(false);
    SetState(PROCESS_AVAILABLE);
}

[[noreturn]] void Reaper() {
    __asm__("cli");

    while (true) {
        while (dying == 0) {
            // TODO: waitFor(100);
        }

        for(size_t i = 0; i < MAX_PROCESSES; i++) {
            if ((loaded && i == 0) || processes[i] == nullptr)
                continue;

            TicketAttemptLock(&creatorlock);
            lockProcess();

            if(processes[i]->GetState() == Process::PROCESS_REAP) {
                SerialPrintf("[ PROC] Killing Process %u (%s)\r\n", i, processes[i]->GetName());
                processes[i]->Destroy();
                delete processes[i];
                processes[i] = nullptr;
                dying--;
                TicketUnlock(&creatorlock);
            }
            unlockProcess();
        }

        // TODO: waitFor(100);
    }
}

void ProcessManager::InitKernelProcess(function_t EntryPoint) {
    SerialPrintf("[ PROC] Initializing process manager\r\n");

    processes = reinterpret_cast<Process**>(kmalloc(sizeof(Process*) * MAX_PROCESSES * PAGE_SIZE));
    Process::SetCurrent(nullptr);

    for (size_t i = 0; i < MAX_PROCESSES; i++) {
        processes[i] = nullptr;
    }

    SerialPrintf("[ PROC] Creating system processes\r\n");
    CreateProcess(NullProcess, true, "testproc", false);
    CreateProcess(EntryPoint, true, "kernel", false);
    CreateProcess(Reaper, true, "reaper", false);

    loaded = true;
    unlockProcess();
    locked = 0;

}

int64_t findFreeProcessSlot() {
    for (size_t i = lastProcess; i < MAX_PROCESSES; i++) {
        if ((loaded && i == 0) || processes[i] != nullptr)
            continue;
        if (processes[i] == nullptr) {
            lastProcess = i + 1;
            return i;
        }
    }

    if (lastProcess == 0)
        return -1;
    else {
        lastProcess = 0;
        return findFreeProcessSlot();
    }
}

Process* ProcessManager::CreateProcessInternal(const char* name, function_t entry, bool userspace) {
    int64_t toAdd = findFreeProcessSlot();
    if (toAdd == -1) {
        SerialPrintf("[ PROC] Unable to create new process %s; OUT_OF_SLOTS\r\n", name);
        return nullptr;
    }
    SerialPrintf("[ PROC] New process: %u, name %s, %s userspace\r\n", nextPID, name, userspace ? "is" : "is not");

    processes[toAdd] = new Process(name, toAdd, nextPID++, (size_t) entry, userspace);
    processes[toAdd]->SetState(Process::PROCESS_NOT_STARTED);
    return processes[toAdd];
}


void ProcessManager::InitProcessPagetable(Process* proc, bool Userspace) {
    if (Userspace)
        proc->GetHeader()->AddressSpace = new address_space_t { NEW_TICKETLOCK(), 0 };
    else
        proc->GetHeader()->AddressSpace = Core::GetCore(Device::APIC::driver->GetCurrentCore())->AddressSpace;
}

void ProcessManager::InitProcessArch(Process* proc) {
    UNUSED(proc);
    // TODO
}

void ProcessManager::InitProcessData(Process* proc, const char* name, bool userspace, char**argv, size_t argc, function_t entry) {
    UNUSED(name);
    UNUSED(argc);
    UNUSED(argv);
    UNUSED(entry);

    InitProcessPagetable(proc, userspace);
    // TODO: Stack, VFS
    InitProcessArch(proc);
}


Process* ProcessManager::CreateProcess(function_t EntryPoint, bool StartImmediately, const char* Name, bool Userspace, size_t TargetCore, size_t argc, char** argv) {
    Process* toAdd = CreateProcessInternal(Name, EntryPoint, Userspace);
    if (toAdd == nullptr) {
        SerialPrintf("[ PROC] Unable to create new process, terminating\r\n");
        return nullptr;
    }

    SerialPrintf("[ PROC] Starting new process %u (%s)\r\n", toAdd->GetPID(), toAdd->GetName());

    toAdd->SetCore(HandleRequest(TargetCore));
    InitProcessData(toAdd, Name, Userspace, argv, argc, EntryPoint);
    toAdd->SetParent(toAdd->GetPID());

    if (StartImmediately || Process::Current() == nullptr) {
        toAdd->SetState(Process::PROCESS_WAITING);
        Process::SetCurrent(toAdd);
    }

    return toAdd;
}

Process* ProcessManager::GetNextToRun(size_t Current) {
    if (locked)
        return Process::Current();

    size_t CoreID = Device::APIC::driver->GetCurrentCore();
    for (size_t i = Current + 1; i < MAX_PROCESSES; i++) {
        if (processes[i] != nullptr && processes[i]->CanRun(CoreID))
            return processes[i];
    }

    // If there's no open slots AFTER this process, check from the start.
    if (Current != 0)
        return GetNextToRun(0);

    // If we get here, there's no slots AND we're 0. No processes left.
    SerialPrintf("[ PROC] Scheduler Error!\r\n[ PROC] Core %u has no work to do.\r\n[ PROC] Last task was %s\r\n", Process::Current()->GetName());

    for(;;) {}
    return nullptr;
}

size_t ProcessManager::SchedulerInterrupt(INTERRUPT_FRAME* CurrentFrame, bool ForceSwitch) {

    SerialPrintf("[ PROC] Scheduler called\r\n");

    if (locked)
        return (size_t) CurrentFrame;

    if (ForceSwitch) {
        for (size_t i = 1; i < MAX_PROCESSES; i++) {
            if (processes[i] != nullptr && processes[i]->IsSleeping()) {
                processes[i]->DecreaseSleep(1);
            }
        }

        NotifyAllCores();
    }

    Process* i;
    if (Process::Current() == nullptr)
        i = GetNextToRun(0);
    else
        i = GetNextToRun(Process::Current()->GetKPID());

    if (i == 0)
        return (size_t) CurrentFrame;

    return SwitchContext(CurrentFrame, i);
}

Process* Process::FromName(const char* name) {
    for (size_t i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i] != nullptr) {
            if (strcmp((char*) name, processes[i]->GetName()))
                return processes[i];
        }
    }

    return nullptr;
}

Process* Process::FromPID(size_t PID) {
    for (size_t i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i] != nullptr) {
            if (processes[i]->GetPID() == PID)
                return processes[i];
        }
    }

    return nullptr;
}

void ProcessManager::Sleep(size_t Count) {
    lockProcess();
    Process::Current()->IncreaseSleep(Count);
    unlockProcess();
    yield();
}

void ProcessManager::Sleep(size_t Count, size_t PID) {
    lockProcess();
    Process::FromPID(PID)->IncreaseSleep(Count);
    unlockProcess();
}

void ProcessManager::Kill(size_t PID, int Code) {
    UNUSED(Code);
    Process* target = Process::FromPID(PID);
    if (target == nullptr) {
        SerialPrintf("[ PROC] Attempted to kill invalid process %u\r\n", PID);
        return;
    }

    lockProcess();
    target->Kill();
    dying++;
    unlockProcess();
}

[[noreturn]] void ProcessManager::Kill(int Code) {
    lockProcess();
    __asm__ __volatile__("cli");

    DeadProcessData data = { Process::Current()->GetPID(), 1, (uint32_t) Code };
    deadProcesses.emplace_back(data);
    Process::Current()->Kill();
    dying++;
    unlockProcess();

    __asm__ __volatile__("sti");

    // Wait until we're reaped
    while (true)
        yield();
}

void ProcessManager::NotifyAllCores() {
    if (Device::APIC::driver->GetCurrentCore() == 0) {
        for (size_t i = 0; i <= CoreCount; i++) {
            if (i != (size_t) Device::APIC::driver->GetCurrentCore())
                Device::APIC::driver->SendInterCoreInterrupt(i, 100);
        }
    }
}

void ProcessManager::MapThreadMemory(Process* proc, size_t from, size_t to, size_t length) {
    for (size_t i = 0; i < length; i++) {
        MapVirtualPage(proc->GetHeader()->AddressSpace, from + (i * PAGE_SIZE), to + (i * PAGE_SIZE), 3);
    }
}

void ProcessManager::GetStatus(size_t PID, int* ReturnVal, size_t* StatusVal) {
    for (size_t i = 0; i < deadProcesses.size(); i++) {
        if (deadProcesses[i].PID == PID) {
            *ReturnVal = deadProcesses[i].ReturnCode;
            *StatusVal = -2;
            return;
        }
    }

    Process* proc = Process::FromPID(PID);
    *StatusVal = proc->GetState() != Process::PROCESS_RUNNING;
    *ReturnVal = -250;
}

void ProcessManager::SwitchContextInternal(Process* next) {
    // TODO: set TSS Stack

    Core::GetCore(Device::APIC::driver->GetCurrentCore())->AddressSpace = next->GetHeader()->AddressSpace;
    WriteControlRegister(3, FROM_DIRECT((size_t)next->GetHeader()->AddressSpace->PML4));
}

size_t ProcessManager::SwitchContext(INTERRUPT_FRAME* frame, Process* NextProcess) {
    if (locked)
        return (size_t) frame;

    if (NextProcess == nullptr)
        return (size_t) frame;

    if (Process::Current() != nullptr) {
        if (Process::Current()->GetState() != Process::PROCESS_REAP) {
            Process::Current()->SetState(Process::PROCESS_WAITING);
            Process::Current()->GetHeader()->RSP = (size_t) frame;
            Process::Current()->GetHeader()->ContextFrame = *frame;
            // TODO: Save SSE Context
        }
    }

    NextProcess->SetState(Process::PROCESS_RUNNING);
    SerialPrintf("[ PROC] Switching to process %u (%s)\r\n", NextProcess->GetPID(), NextProcess->GetName());
    Process::SetCurrent(NextProcess);
    *frame = Process::Current()->GetHeader()->ContextFrame;

    // TODO: Load SSE context

    // TODO: New process has 0 mappings, so triple faults.
    // TODO: copy page tables?
    SwitchContextInternal(NextProcess);

    return NextProcess->GetHeader()->RSP;
}

void* Process::AllocateProcessSpace(size_t Bytes) {
    return (void*) ProcessMemory.allocate(Bytes);
}

size_t Process::FreeProcessSpace(size_t Address, size_t Bytes) {
    if (OwnsAddress(Address, Bytes)) {
        ProcessMemory.setFree(Address, Bytes);
        return Bytes;
    } else {
        return 0;
    }
}

bool Process::OwnsAddress(size_t Address, size_t Bytes) {
    if (((Address + Bytes) * PAGE_SIZE) > USERSPACE_MEM_SIZE)
        return false;

    for (size_t i = Address; i < Address + Bytes; i++) {
        if (!ProcessMemory.get(i))
            return false;
    }

    return true;
}

size_t ProcessManager::HandleRequest(size_t CPU) {
    if (CPU == USE_CURRENT_CPU) {
        return Device::APIC::driver->GetCurrentCore();
    } else if (CPU == BALANCE_CPUS) {
        lastSelectedCPU++;
        if (lastSelectedCPU > CoreCount)
            lastSelectedCPU = 0;
        return lastSelectedCPU;
    } else {
        return CPU;
    }
}