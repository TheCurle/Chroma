#pragma once
#include <driver/generic/device.h>
#include <kernel/system/acpi/madt.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

namespace Device {

    /**
     * Contains the driver definition for the Advanced Programmable Interrupt Controller - APIC.
     * Each processor core contains at least one APIC, and they have a unique identifier,
     * so they can be used to identify cores along with producing timed interrupts.
     * 
     * However, this is only possible because this driver is loaded once per system.
     * This is a singleton class.
     * 
     * The primary functionality of the APIC is to send an interrupt to the given core after the specified time is up.
     */

    class APIC : public IODevice {
    private:
        // Metadata of IOAPICs.
        struct IOAPICMeta {
            uint8_t Version;
            uint8_t Reserved;
            uint8_t MaxRedirect;
            uint8_t Reserved2;
        };

        // The address in physical memory that refers to this device.
        void* Address;
        // Whether this APIC driver is ready for processing.
        bool Ready = false;

        ACPI::MADT::IOAPICEntry** IOAPICs;
        ACPI::MADT::ISOEntry** ISOs;
        IOAPICMeta* Meta;

        // The internal implementation of Set. Handles raw redirects at the hardware level.
        void SetInternal(uint8_t Vector, uint32_t TargetGSI, uint16_t Flags, int Core, int Status);

    public:

        APIC();

        // The register locations of the APIC device.
        enum Registers {
            LAPIC_ID = 0x20, // ID of the APIC
            EOI = 0xB0,      // Acknowledge
            SIVR = 0xF0,     // Spurious Interrupt Vector Register
            ICR1 = 0x300,    // Interrupt Command Register Lower
            ICR2 = 0x310,    // Interrupt Command Register Higher
            LVT = 0x320,     // Local Vector Table
            LINT1 = 0x350,   // Local Interrupt ID
            LINT2 = 0x360,   // Local Interrupt ID
            TIMER_DIVISOR = 0x3E0, // Frequency divisor for the timer.
            TIMER_INIT = 0x380,    // Initializer for the timer.
            TIMER_CURRENT = 0x390, // Current "tick count" of the timer.
        };

        // The instance of this singleton class.
        static APIC* driver;

        const char* GetName() const override {
            return "APIC";
        }

        // Dump all available information to the log (or to stdout eventually)
        void LogDump();

        // Move all structs and internal handlers to a new address.
        void Move(size_t NewAddress);

        // Prepare the APICs for use.
        void Init() override;
        // Load all data the APICs need.
        void LoadInterruptSystem();
        // Set the APICs ready for use.
        void Enable();
        // Check whether the APICs are ready for use.
        bool IsReady();

        // Prepare a core for use with interrupts.
        void PreinitializeCore(int Core);
        // Set a core as available to use interrupts.
        void InitializeCore(int Core, size_t EntryPoint);

        // Check what core ID is currently running.
        int GetCurrentCore();

        uint32_t ReadRegister(uint32_t Register) override;
        void WriteRegister(uint32_t Register, uint32_t Data) override;

        uint32_t ReadIO(size_t Base, uint32_t Register) override;
        void WriteIO(size_t Base, uint32_t Register, uint32_t Data) override;

        // Send a specified interrupt to another core.
        void SendInterCoreInterrupt(int Core, uint32_t Interrupt);
        // Tell the APIC that the interrupt is acknowledged. EOI = End Of Interrupt.
        void SendEOI();

        // Tell the APIC that the specified IRQ should come to this core.
        void Set(int Core, uint8_t IRQ, int status);

        // Write data into an inter-core interrupt.
        void WriteICI(size_t Data);
    
        // Check what the maximum allowed interrupt redirect is.
        uint32_t GetMaxRedirect(uint32_t APIC);

    };
};