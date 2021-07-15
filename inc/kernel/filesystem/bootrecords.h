#pragma once
#include <stdint.h>
#include <stddef.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

/**
 * @brief Abstract class for partition table implementations.
 * The partition table implements a way to store and handle the boot record for any system.
 * ! Do not instantiate!
 */
class BasePartitionTable {
public:

    // Initialize the partition object
    virtual void Init() = 0;

    // Get the amount of partitions in the table
    virtual uint8_t GetPartitions() = 0;

    // Get the offset which, when added to the start of the disk, will provide the start of the given partition.
    virtual size_t GetPartitionStart(uint8_t PartitionID) = 0;
    // Get the length of the given partition on disk.
    virtual size_t GetPartitionSize(uint8_t PartitionID) = 0;

private:

    static BasePartitionTable* Primary;
};


/**
 * @brief A BasePartitionTable implementation for MBR (BIOS) systems.
 */
class MBRPartitionTable : public BasePartitionTable {
public:

    // Initialize the partition object
    virtual void Init() override;

    // Get the amount of partitions in the table
    virtual uint8_t GetPartitions() override;

    // Get the offset which, when added to the start of the disk, will provide the start of the given partition.
    virtual size_t GetPartitionStart(uint8_t PartitionID) override;
    // Get the length of the given partition on disk.
    virtual size_t GetPartitionSize(uint8_t PartitionID) override;

private:

    // An MBR Partition Table Entry
    struct MBREntry {
        uint8_t Status;         // 0x80 for "active", 0x00 for inactive, all other values are invalid.
        uint8_t CHS_First[3];   // Cylinder-Head-Sector address of the first sector in this partition.
        uint8_t Type;           // Partition type. https://en.wikipedia.org/wiki/Partition_type
        uint8_t CHS_Last[3];    // Cylinder-Head-Sector address of the last sector in this partition.
        uint32_t LBA_First;     // LBA of the first sector in this partition
        uint32_t SectorCount;   // Total count of sectors in this partition
    } __attribute__((packed));

    MBREntry* Entries;
};