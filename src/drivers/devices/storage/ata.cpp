#include <kernel/system/io.h>
#include <lainlib/mutex/ticketlock.h>
#include <driver/storage/ata.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/
using namespace Device;

// Internal storage.
bool IRQWaiting = false;
ticketlock_t ATALock;

void ATADevice::Init() {
    if(!HasATA()) {
        SerialPrintf("[  ATA] No ATA device found.\r\n");
        return;
    }

    SelectedDrive = ATAType::PRIMARY_MASTER;

    WritePort(ATAAddress::PRIMARY_DCR, 0x04, 1);
    WritePort(ATAAddress::PRIMARY_DCR, 0x00, 1);

    uint8_t Status = ATACommandRead(true, ATARegister::COMMAND_STATUS);
    while((Status & 0x80) && !(Status & 0x1)) { // While BSY && !ERR
        Status = ATACommandRead(true, ATARegister::COMMAND_STATUS);
    }
}

void ATADevice::HandleIRQ(size_t IRQID) {
    if(IRQWaiting)
        IRQWaiting = false;
    
    if(IRQID == 14)
        ReadPort(PRIMARY + COMMAND_STATUS, 1);

    if(IRQID == 15)
        ReadPort(SECONDARY + COMMAND_STATUS, 1);
}

void ATADevice::ATACommandWrite(bool Primary, uint16_t Register, uint16_t Data) {
    WritePort((Primary ? PRIMARY : SECONDARY) + Register, Data, 2);
}

uint16_t ATADevice::ATACommandRead(bool Primary, uint16_t Register) {
    return ReadPort((Primary ? PRIMARY : SECONDARY) + Register, 2);
}

bool ATADevice::GetStatus() {
    uint8_t Status = ATACommandRead(true, COMMAND_STATUS);
    while((Status & 0x80) && !(Status & 0x8)) { // While BSY && !DRQ
        Status = ATACommandRead(true, COMMAND_STATUS);
    }
    return true;
}

bool ATADevice::HasATA() {
    // https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command
    ATACommandWrite(true, SECTOR_COUNT, 0);
    ATACommandWrite(true, LBA0, 0);
    ATACommandWrite(true, LBA1, 0);
    ATACommandWrite(true, LBA2, 0);
    ATACommandWrite(true, COMMAND_STATUS, IDENTIFY);

    uint8_t Status = ATACommandRead(true, COMMAND_STATUS);
    if(Status == 0)
        return false;
    
    // TODO: ATAPI weirdness
    return true;
}

void ATADevice::ReadData(size_t Address, size_t Length, uint8_t* Buffer) {
    TicketAttemptLock(&ATALock);
    IRQWaiting = true;

    ATACommandWrite(true, SELECTOR, 0x40 | 0xE0);
    ATACommandWrite(true, ERROR_FEATURE, 0);
    
    ATACommandWrite(true, SECTOR_COUNT, Length >> 8);
    ATACommandWrite(true, LBA0, (uint8_t)(Address >> 24));
    ATACommandWrite(true, LBA1, (uint8_t)(Address >> 32));
    ATACommandWrite(true, LBA2, (uint8_t)(Address >> 40));
    
    ATACommandWrite(true, SECTOR_COUNT, Length);
    ATACommandWrite(true, LBA0, (uint8_t)(Address));
    ATACommandWrite(true, LBA1, (uint8_t)(Address >> 8));
    ATACommandWrite(true, LBA2, (uint8_t)(Address >> 16));
    
    ATACommandWrite(true, COMMAND_STATUS, READ);

    Wait(5);

    size_t Counter = Length;
    size_t Offset = 0;

    while(Counter--) {
        Wait(5);
        size_t Status = Spinlock();
        
        if((Status & 0x1) || (Status & 0x20)) {
            Wait(5);

            uint8_t Error = ATACommandRead(true, ERROR_FEATURE);
            SerialPrintf("[  ATA] Logged error %d.\r\n", Error);
            TicketUnlock(&ATALock);
            return;
        }

        for(uint16_t i = 0; i < 256; i++)
            *((unsigned short*) Buffer + (Offset + i)) = ATACommandRead(true, DATA);
        
        Offset += 256;
    }

    TicketUnlock(&ATALock);
    return;
}