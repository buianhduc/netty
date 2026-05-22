//
// Created by Đức Bùi on 19/5/2026.
//

#ifndef NES_EMULATOR_PPUREGISTER_H
#define NES_EMULATOR_PPUREGISTER_H
#include "internal/flag.hpp"


class PPURegister : public BitFlags{
    bool readable = false;
    bool writable = false;
    public:
    PPURegister(bool readable, bool writeable, BitFlags init_value = 0) :
    BitFlags(init_value), readable(readable), writable(writeable) {}
    PPURegister(): BitFlags(0x0) {}
    ~PPURegister()  = default;

};


#endif //NES_EMULATOR_PPUREGISTER_H
