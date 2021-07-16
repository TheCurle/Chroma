#pragma once
#include <driver/generic/device.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

namespace Device {

    /**
     * @brief An implementation of a generic Parallel ATA device driver.
     * Handles managing the state of the drive, reading and writing data, etc.
     * 
     * ATA Devices are currently READ ONLY.
     * Writing to ATA is currently a NO-OP.
     */
    class ATADevice : public GenericStorage {
    public:

        /***************************/
        /*       DEFINITIONS       */
        /***************************/

        // Base addresses for port I/O.
        enum ATAAddress {
            PRIMARY = 0x1F0,
            SECONDARY = 0x170,
            PRIMARY_DCR = 0x3F6,
            SECONDARY_DCR = 0x376
        };

        // Used by the IDENTIFY command to tell the OS what type of drive this is.
        enum ATAType {
            PRIMARY_MASTER = 1,
            PRIMARY_SLAVE = 2,
            SECONDARY_MASTER = 3,
            SECONDARY_SLAVE = 4
        };

        // A sequential list of registers that can be referenced by adding to a base address.
        enum ATARegister {
            DATA,
            ERROR_FEATURE,
            SECTOR_COUNT,
            LBA0,
            LBA1,
            LBA2,
            SELECTOR,
            COMMAND_STATUS,
            SECTOR_COUNT_2,
            LBA3,
            LBA4,
            LBA5,
            CONTROL_STATUS,
            DEVICE_ADDRESS
        };

        // The common data responses that a drive can respond with.
        enum ATAData {
            TYPE_MASTER = 0xA0,
            TYPE_SLAVE = 0xB0,
            STATE_BUSY = 0x80,
            STATE_FAULT = 0x20,
            STATE_DRQ = 0x8,
            STATE_ERROR = 0x1
        };

        // The commands that we can send to the drive.
        enum ATACommand {
            IDENTIFY = 0xEC,
            READ = 0x24
        };

        /***************************/
        /*        FUNCTIONS        */
        /***************************/

        // Initialize the drive.
        void Init();

        // Does this system have an ATA connection?
        static bool HasATA();


        // Pause for the given amount of read cycles.
        inline void Wait(uint8_t Cycles) {
            for(uint8_t i = 0; i < Cycles; i++)
                ATACommandRead(true, ATARegister::COMMAND_STATUS);
        }

        // Block until the drive returns data.
        inline uint8_t Spinlock() {
            uint8_t Status = ATACommandRead(true, ATARegister::COMMAND_STATUS);
            while(Status & 0x80) {
                Status = ATACommandRead(true, ATARegister::COMMAND_STATUS);
                __asm__ __volatile__("pause");
            }
            return Status;
        }

        // Handle an incoming IRQ from the drive.
        void HandleIRQ(size_t IRQID);

        // Read data from the drive.
        GenericStorage::Status Read(uint8_t* Data, size_t Length, size_t Start) override {
            ReadData(Start, Length, Data);
            return GenericStorage::Status::OKAY;
        }

        // NOT IMPLEMENTED. WILL OVERWRITE THE DATA BUFFER WITH THE CONTENTS OF THE DISK AT Start.
        GenericStorage::Status Write(uint8_t* Data, size_t Length, size_t Start) override {
            ReadData(Start, Length, Data);
            return GenericStorage::Status::OKAY;
        }

        const char* GetName() const final {
            return "PATA-IDE";
        }

    private:

        // Used to reduce redundant Selections
        ATAType SelectedDrive;

        // Write data to the ATA command registers.
        static void ATACommandWrite(bool Primary, uint16_t Register, uint16_t Data);
        // Read a value from the ATA command register.
        static uint16_t ATACommandRead(bool Primary, uint16_t Register);

        // Read data from the drive into a buffer.
        void ReadData(size_t Location, size_t Length, uint8_t* Buffer);

        // Get the current status of the driver.
        static bool GetStatus();
    };
};