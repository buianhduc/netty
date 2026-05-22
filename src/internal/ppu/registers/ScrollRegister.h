//
// Created by Đức Bùi on 19/5/2026.
//

#ifndef NES_EMULATOR_SCROLLREGISTER_H
#define NES_EMULATOR_SCROLLREGISTER_H
#include <cstdint>


class ScrollRegister {
    uint8_t x_scroll_ = 0;
    uint8_t y_scroll_ = 0;
    bool x_ptr_ = true;

public:
    void update(uint8_t data);
    void update(uint8_t data, bool first_write);
    void reset_latch();

    [[nodiscard]] uint8_t x_scroll() const;
    [[nodiscard]] uint8_t y_scroll() const;
};


#endif //NES_EMULATOR_SCROLLREGISTER_H
