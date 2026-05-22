//
// Created by Đức Bùi on 21/5/2026.
//

#ifndef NES_EMULATOR_JOYPAD_H
#define NES_EMULATOR_JOYPAD_H
#include <cstdint>


enum JoypadButton: uint8_t {
    RIGHT = 1 << 7,
    LEFT = 1 << 6,
    DOWN = 1 << 5,
    UP = 1 << 4,
    START = 1 << 3,
    SELECT = 1 << 2,
    BUTTON_B = 1 << 1,
    BUTTON_A = 1 << 0,
    NONE = 0
};
class Joypad {
    bool strobe = false;
    uint8_t button_index = 0;
    uint8_t button_status = NONE;
    public:
    Joypad() = default;
    ~Joypad() = default;
    void write(uint8_t data);

    uint8_t read();
    void set_button_pressed_status(JoypadButton button, bool pressed);

};


inline void Joypad::write(uint8_t data) {
    strobe = (data & 1) == 1;
    if (strobe) {
        button_index = 0;
    }
}

inline uint8_t Joypad::read() {
    if (button_index > 7) {
        return 1;
    }
    const auto response = static_cast<uint8_t>((button_status >> button_index) & 1u);
    if (!strobe) {
        button_index += 1;
    }
    return response;

}

inline void Joypad::
set_button_pressed_status(JoypadButton button, bool pressed) {
    if (pressed) {
        button_status |= button;
    } else {
        button_status &= static_cast<uint8_t>(~button);
    }
}
#endif //NES_EMULATOR_JOYPAD_H
