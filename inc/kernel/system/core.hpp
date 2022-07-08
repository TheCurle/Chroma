#pragma once

#include <kernel/constants.hpp>
#include <kernel/system/descriptors.h>
#include <kernel/system/memory.h>
#include <stddef.h>
#include <stdint.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

/**
 * The relevant information contained within a stack trace - each call consists of the
 * contained information. By navigating the RBP until the base is found, a full backwards call stack can be determined.
 */
struct StackFrame {
    StackFrame* rbp;
    size_t rip;
};

/**
 * Contains the definitions required to define and manage a single physical processing core.
 * These include; active GDT and IDT, TSS segments, saved stacks for execution and syscalls.
 * There are some utility functions for saving and loading extended registers, as well as
 *  for identifying individual Cores in a running system.
 * 
 */
class Core {
   public:
    Core() {}
    Core(size_t LAPIC, size_t ID);

    size_t ID;
    size_t LocalAPIC;

    address_space_t* AddressSpace;

    uint8_t* SyscallStack;
    size_t StackAddress;
    uint8_t StackData[Constants::Core::STACK_SIZE];

    IDT CoreIDT;
    GDT CoreGDT;
    TSS64 CoreTSS;

    void LoadExtraRegisters(uint8_t* Data);
    void SaveExtraRegisters(uint8_t* Data);

    void StackTrace(size_t Cycles);

    static Core* GetCurrent() {
        size_t CoreID = 0;
        __asm__ __volatile__("mov %0, %%fs\n" : "=r"(CoreID) : :);
        return &Processors[CoreID];
    }

    static Core* GetCore(int ID) { return &Processors[ID]; }

    static void Init();

   private:
    static Core Processors[];

    // Initialization vectors for all new cores.
    // Numbers derived from boot.h space.
    // Specifically, the bootloader ROM. 
    enum Initialization {
        PAGETABLES = 0x600,
        STARTUP = 0x620,
        STACK = 0x670,
        GDT = 0x680,
        IDT = 0x690
    };

    void Bootstrap();
    void SetupData(size_t ID);

};
