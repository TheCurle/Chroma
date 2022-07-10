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
    Core(){}
    Core(size_t LAPIC, size_t ID);

    size_t ID = 0;
    size_t LocalAPIC = 0;

    address_space_t* AddressSpace = nullptr;

    uint8_t* SyscallStack = 0;
    size_t StackAddress = 0;
    uint8_t StackData[Constants::Core::STACK_SIZE] = { 0 };

    IDT CoreIDT;
    GDT CoreGDT;
    TSS64 CoreTSS;

    void LoadExtraRegisters(uint8_t* Data);
    void SaveExtraRegisters(uint8_t* Data);

    void StackTrace(size_t Cycles);

    static Core* GetCurrent() {
        size_t CoreID = 0;
        __asm__ __volatile__("mov %0, %%fs\n" : "=r"(CoreID) : :);
        return Processors[CoreID];
    }

    static Core* GetCore(int ID) { return Processors[ID]; }

    static void Init();

   private:
    static Core* Processors[];

    void Bootstrap();
    void SetupData(size_t ID);

};
