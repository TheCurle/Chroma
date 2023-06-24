#pragma once
#include <driver/generic/device.h>

/************************
 *** Team Kitty, 2022 ***
 ***     Chroma       ***
 ***********************/

extern char keys[128];

namespace Device {
    class PS2Keyboard : public GenericKeyboard {
        bool keyStates[128];
        uint8_t lastPress = 0;
        uint8_t* callback;

    public:
        PS2Keyboard();

        // The instance of this singleton class.
        static PS2Keyboard* driver;

        const char* GetName() const {
            return "PS2 Keyboard";
        };

        void Init();
        void InterruptHandler();

        void setState(bool state, uint8_t key);
        bool isPressed(uint8_t key) const;

        uint8_t getLastPress() const {
            return keys[lastPress];
        }

        void SetCallback(uint32_t*);
    };
}