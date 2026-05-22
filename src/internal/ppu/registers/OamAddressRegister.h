//
// Created by Đức Bùi on 19/5/2026.
//

#ifndef NES_EMULATOR_OAMADDRESSREGISTER_H
#define NES_EMULATOR_OAMADDRESSREGISTER_H
#include <cstdint>


class OamAddressRegister {
    uint8_t value_ = 0;

public:
    void update(uint8_t data);
    void increment();
    [[nodiscard]] uint8_t get() const;
};


#endif //NES_EMULATOR_OAMADDRESSREGISTER_H
