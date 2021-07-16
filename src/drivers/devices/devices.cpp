#include <driver/generic/device.h>
#include <kernel/system/io.h>

/************************
 *** Team Kitty, 2021 ***
 ***     Chroma       ***
 ***********************/

// TODO: increase this
#define MAX_DEVICES 8
// TODO: increase this
#define MAX_STORAGE_DEVICES 4

// Internal storage. TODO: Turn this into some form of search tree structure.
Device::GenericDevice* DevicesArray[MAX_DEVICES];
// Internal storage. Index into the above array.
size_t CurrentDevice = 0;

// Internal storage. TODO: Turn this into some form of search tree structure.
Device::GenericStorage* StorageDevicesArray[MAX_STORAGE_DEVICES];
// Internal storage. Index into the above array.
size_t CurrentStorageDevice = 0;

// Internal storage. TODO: Make this not a pain to maintain
const char* DeviceNames[] = { "Storage", "Keyboard", "Networking" };


// Add a device pointer to the managed list.
void Device::RegisterDevice(Device::GenericDevice* Device) {
    DevicesArray[CurrentDevice] = Device;
    Device->DeviceID = CurrentDevice;
    CurrentDevice++;
    SerialPrintf("[DEVICE] Registered device %d called %s of type %s\r\n", CurrentDevice - 1, Device->GetName(), DeviceNames[Device->GetType()]);
}

// Retrieve a device pointer from the managed list.
Device::GenericDevice* Device::GetDevice(size_t ID) {
    return DevicesArray[ID];
}

void Device::RegisterStorageDevice(Device::GenericStorage* Device) {
    RegisterDevice(Device);
    StorageDevicesArray[CurrentStorageDevice] = Device;
    CurrentStorageDevice++;
}

Device::GenericStorage* Device::GetStorageDevice(size_t ID) {
    return StorageDevicesArray[ID];
}

// Get the count of registered devices.
size_t Device::GetTotalDevices() { return CurrentDevice; }

template <typename T>
// Get the first registered instance of a specific type of device
T* Device::FindDevice() {
    for(size_t i = 0; i < CurrentDevice; i++)
        if(DevicesArray[i]->GetType() == T::GetRootType())
            return static_cast<T*>(DevicesArray[i]);

    SerialPrintf("[DEVICE] Warning: Unable to find a %s device.\r\n", DeviceNames[T::GetRootType()]);
    return static_cast<T*>(nullptr);
}

// Get the first registered instance of a Storage device
template Device::GenericStorage* Device::FindDevice();