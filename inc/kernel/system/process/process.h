#pragma once

#include <kernel/system/descriptors.h>
#include <kernel/system/memory.h>
#include <kernel/system/interrupts.h>
#include <kernel/system/process/process.h>
#include <lainlib/vector/bitmap.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

#define MAX_CORES 8
#define MAX_PROCESSES 128
#define PROCESS_STACK 65535

#define USE_CURRENT_CPU ((size_t)-1)
#define BALANCE_CPUS ((size_t)-2)


typedef void (* function_t)();

/**
 * @brief All the data a process needs.
 *
 * Contains all the process-specific structs.
 * Lots of private members, out of necessity
 */
class Process {
public:
    // Tells the scheduler, and system at large, what the current state of the process is
    enum ProcessState {
        PROCESS_AVAILABLE,  // Process is ready to be scheduled
        PROCESS_RUNNING,    // Process is active on the CPU
        PROCESS_WAITING,    // Process is blocked by external activity
        PROCESS_CRASH,      // Process has encountered an error and needs to be culled
        PROCESS_NOT_STARTED,// Process is waiting for bootstrap
        PROCESS_REAP        // Process wants to die
    };

    // The process' buffers, for moving data in and out of the system
    enum BufferTypes {
        STDOUT,
        STDIN,
        STDERR
    };

    // The data that actually represents one of the above buffers.
    struct Buffer {
        uint8_t* Data;
        size_t Length;
        size_t LengthAllocated;
        uint8_t Type; // An entry of BufferTypes.
    } __attribute__((packed));

    // A packet used for IPC
    struct ProcessMessage {
        uint8_t MessageID;
        size_t Source;      // Originating PID
        size_t Destination; // Target PID
        size_t Content;     // Pointer to the data.
        size_t Length;      // Size of the data. End is Content + Length
        size_t Response;
        bool Read;          // True if message has been read
        bool Free;          // True if memory can be reused for something else
    } __attribute__((packed));

    // Important information about the process.
    // Its' stack and stack pointer, plus the page tables.
    struct ProcessHeader {
        uint8_t* Stack;
        size_t StackSize;

        size_t RSP;
        uint8_t* SSE;

        address_space_t* AddressSpace;
        INTERRUPT_FRAME ContextFrame;
    };

private:
    ProcessHeader Header;
    ProcessState State;

    bool User;        // Is this process originated in userspace?
    bool System;      // Was this process started by the system (ie. not user interaction)

    size_t UniquePID; // Globally Unique ID.
    size_t KernelPID; // If in kernel space, the PID.
    size_t ParentPID; // If this process was forked, the parent's PID. Otherwise, the kernel.

    char Name[128];
    size_t Entry;     // The entry point. Move execution here to start the process.
    uint8_t Core;

    bool ORS = false;
    bool IsActive = false;
    bool IsInterrupted = false; // True if an interrupt was fired while this process is active

    uint8_t Signals[8]; // Interrupt / IRQ / Signal handlers.
    uint8_t Sleeping;   // 0 if active, else the process is waiting for something. TODO: remove this, use State?

    ProcessMessage* Messages; // A queue of IPC messages.
    size_t LastMessage; // The index of the current message.

    lainlib::bitmap ProcessMemory;

    // TODO: Stack Trace & MFS

public:

    Process(size_t KPID) : State(PROCESS_AVAILABLE), UniquePID(-1), KernelPID(KPID) {
    };

    Process(const char* ProcessName, size_t KPID, size_t UPID, size_t EntryPoint, bool Userspace)
            : User(Userspace), UniquePID(UPID), KernelPID(KPID), Entry(EntryPoint), ORS(false), Sleeping(0),
              LastMessage(0), ProcessMemory(new uint8_t[USERSPACE_MEM_SIZE / PAGE_SIZE / 8], USERSPACE_MEM_SIZE / PAGE_SIZE){

        memcpy((void*) Name, ProcessName, strlen(ProcessName) + 1);
        ProcessMemory.setFree(0, USERSPACE_MEM_SIZE / PAGE_SIZE);
    };

    Process(const Process &Instance) {
        memcpy(this, &Instance, sizeof(Process));
    };

    Process &operator =(const Process &Instance) {
        memcpy(this, &Instance, sizeof(Process));
        return *this;
    };

    /*************************************************************/

    void InitMemory();

    void InitMessages();

    void Kill() {
        State = ProcessState::PROCESS_REAP;
        Sleeping = -1;
    };

    void Destroy();

    void Rename(const char* NewName) {
        memcpy(Name, NewName, strlen(Name) > strlen(NewName) ? strlen(Name) : strlen(NewName));
    }

    void* AllocateProcessSpace(size_t Bytes);

    size_t FreeProcessSpace(size_t Address, size_t Bytes);

    bool OwnsAddress(size_t Address, size_t Bytes);

    /*************************************************************/

    void SetParent(size_t PID) { ParentPID = PID; };

    void SetSystem(bool Status) {
        System = Status;
        if (System && User) {
            // TODO: Log error.
        }
    };

    void SetState(ProcessState NewState) { State = NewState; };

    void SetActive(bool NewState) { IsActive = NewState; };

    void SetCore(size_t CoreID) { Core = CoreID; };

    void IncreaseSleep(size_t Interval) { Sleeping += Interval; };

    void DecreaseSleep(size_t Interval) { Sleeping -= Interval; };

    /*************************************************************/

    ProcessHeader* GetHeader() { return &Header; };

    const char* GetName() const { return Name; };

    size_t GetPID() const { return UniquePID; };

    size_t GetKPID() const { return KernelPID; };

    size_t GetParent() const { return ParentPID; };

    ProcessState GetState() const { return State; };

    bool IsValid() const { return KernelPID != 0; };

    bool IsUsed() const { return (State != ProcessState::PROCESS_AVAILABLE && State != ProcessState::PROCESS_CRASH &&
                                  State != ProcessState::PROCESS_REAP) && IsValid();
    };

    bool IsSleeping() const { return Sleeping; };

    size_t GetSleepCounter() const { return Sleeping; };

    bool CanRun(const size_t CPU) const {
        bool flag = !(ORS && !IsActive);

        return State == ProcessState::PROCESS_WAITING && Core == CPU && KernelPID != 0 && flag && !IsSleeping();
    };

    size_t GetCore() const { return Core; };

    bool IsUserspace() { return User; };

    bool IsSystem() { return System; };


    /*************************************************************/

    static Process* FromName(const char* name);

    static Process* FromPID(size_t PID);

    static Process* Current();

    static void SetCurrent(Process* Target);

};


/**
 * Handles tasks related to processes in general, but that don't need to be
 * directly linked to any one specific process.
 * 
 * Stuff like switching tasks, sleeping, killing, etc.
 */
class ProcessManager {
public:
    TSS64 TSS[MAX_CORES];
    uint32_t CoreCount = 1;

    ProcessManager() {}

    static ProcessManager* instance;

    // Sets internal data, such as the paging tables.
    void SwitchContextInternal(Process* next);

    // Switch to the given process on the current core.
    size_t SwitchContext(INTERRUPT_FRAME* frame, Process* NextProcess);

    // Tell all cores to immediately switch context.
    void NotifyAllCores();

    // Map some pages into the given process' memory.
    void MapThreadMemory(Process* proc, size_t from, size_t to, size_t length);

    // Check what the process is up to.
    void GetStatus(size_t PID, int* ReturnVal, size_t* StatusVal);

    // Kill the current process with the given return code.
    [[noreturn]] static void Kill(int Code);

    // Kill the given process with the given return code.
    void Kill(size_t PID, int Code);

    // Sleep the current process for the given number of milliseconds
    static void Sleep(size_t Count);

    // Sleep the given process for the given number of milliseconds
    void Sleep(size_t Count, size_t PID);

    // Called from an interrupt timer handler. Forces task scheduling to switch.
    size_t SchedulerInterrupt(INTERRUPT_FRAME* CurrentFrame, bool ForceSwitch);

    // Get the next process to run. If current is the ID of the current process, this will emulate round-robin scheduling.
    Process* GetNextToRun(size_t current);

    // Create a Process instance for the given data
    Process* CreateProcessInternal(const char* name, function_t entry, bool userspace);

    // Set up the process ready to run; will be made the active process if StartImmediately is set, or if there is no active process.
    Process* CreateProcess(function_t EntryPoint, bool StartImmediately, const char* Name, bool Userspace, size_t TargetCore = USE_CURRENT_CPU, size_t argc = 0, char** argv = nullptr);

    // Initialize the data that a process needs to run.
    void InitProcessData(Process* proc, const char* name, bool userspace, char**argv, size_t argc, function_t entry);

    // Create a process for the given function, with the given arguments.
    Process* CreateProcess(function_t EntryPoint, size_t argc, char** argv);

    // A helper for creating the kernel process with PID 0.
    void InitKernelProcess(function_t EntryPoint);

    // Set up paging for a new process
    void InitProcessPagetable(Process* proc, bool Userspace);

    // Set up architecture-specific data for a process. AARCH64 maybe?
    void InitProcessArch(Process* proc);

    // Deal with current CPU / CPU load balancing
    size_t HandleRequest(size_t CPU);

    inline static void yield() { __asm __volatile("int $100"); }
};