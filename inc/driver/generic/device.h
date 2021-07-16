#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

/**
 * @brief The device namespace, containing all definitions and classes required to implement and communicate with a device.
 */
namespace Device {

    // The internal typemap of devices used in Chroma.
    enum DeviceType : uint32_t {
        STORAGE = 0,
        KEYBOARD = 1,
        NETWORK = 2,
        UNKNOWN = 0xFFFFFFFF
    };

    // The base class that all devices extend from.
    class GenericDevice {
    public: 
        
        // The internal ID of this device.
        size_t DeviceID;

        virtual ~GenericDevice() = default;

        // Return the string representation of the name of the device.
        // Intended to be overriden by subclasses.
        virtual const char* GetName() const {
            return "Null Device";
        };

        // Return the DeviceType enum value that represents this device.
        // Intended to be overriden by subclasses.
        virtual DeviceType GetType() const {
            return DeviceType::UNKNOWN;
        };
        
        // Return the DeviceType enum value that represents this device.
        // Provided for utility checks.
        static DeviceType GetRootType() {
            return DeviceType::UNKNOWN;
        };
    };

    // TODO: GenericKeyboard
    // TODO: GenericDebugger
    // TODO: GenericNetwork


    // The base class that all storage devices extend from.
    class GenericStorage : public GenericDevice {
    public:

        // The return value of all base operations.
        // Dictates what should happen next.
        enum Status {
            ERROR = 0,
            OKAY = 1
        };

        // This is a storage device.
        DeviceType GetType() const final {
            return DeviceType::STORAGE;
        };

        // Provided for utility checks.
        static DeviceType GetRootType() {
            return DeviceType::STORAGE;
        };

        // TODO: Unaligned read/write
        virtual Status Read(uint8_t* Buffer, size_t Count, size_t Start) = 0;
        virtual Status Write(uint8_t* Data, size_t Count, size_t Start) = 0;
    
    };

    
    // Add a device pointer to the managed list.
    void RegisterDevice(GenericDevice* Dev);
    // Retrieve a device pointer from the managed list.
    GenericDevice* GetDevice(size_t ID);

    // Add a Storage device pointer to the managed list.
    void RegisterStorageDevice(GenericStorage* Dev);
    // Retrieve a Storage device pointer from the managed list.
    GenericStorage* GetStorageDevice(size_t ID);

    // Get the count of registered devices.
    size_t GetTotalDevices();

    template <typename T>
    // Get the first registered instance of a specific type of device
    extern T* FindDevice();

};