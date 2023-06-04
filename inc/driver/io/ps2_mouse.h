#pragma once
#include <driver/generic/device.h>

/************************
 *** Team Kitty, 2023 ***
 ***     Chroma       ***
 ***********************/

namespace Device {
    class PS2Mouse : public GenericDevice {
        int32_t x, y;
        uint8_t xOffset, yOffset;
        bool buttonStates[3];

        uint32_t byteRead = 0;
        uint8_t packetData[3] = { 0 };

    public:
        PS2Mouse();

        // The instance of this singleton class.
        static PS2Mouse* driver;

        const char* GetName() const {
            return "PS2 Mouse";
        };

        // This is an internal register device.
        DeviceType GetType() const final {
            return DeviceType::INTERFACE;
        }

        void Init();

        uint32_t getX() const { return x; }
        uint32_t getY() const { return y; }

        uint32_t getXOffset() const { return xOffset; }
        uint32_t getYOffset() const { return yOffset; }

        void handlePacket();
        bool isPressed(uint8_t button) const { return buttonStates[button]; }
    };
}