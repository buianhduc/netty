//
// Created by Đức Bùi on 19/5/2026.
//

#ifndef NES_EMULATOR_OAMDATAREGISTER_H
#define NES_EMULATOR_OAMDATAREGISTER_H
#include <cstdint>

#include "PPURegister.h"


class OamDataRegister : PPURegister {
public:
    OamDataRegister() : PPURegister(true, true) {}
};


#endif //NES_EMULATOR_OAMDATAREGISTER_H
