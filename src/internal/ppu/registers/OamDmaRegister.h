//
// Created by Đức Bùi on 19/5/2026.
//

#ifndef NES_EMULATOR_OAMDMAREGISTER_H
#define NES_EMULATOR_OAMDMAREGISTER_H
#include <cstdint>


class OamDmaRegister {
    uint8_t page_ = 0;

public:
    void update(uint8_t data);
    [[nodiscard]] uint16_t source_address() const;
};


#endif //NES_EMULATOR_OAMDMAREGISTER_H
