//
// Created by Đức Bùi on 19/5/2026.
//

#ifndef NES_EMULATOR_DATAREGISTER_H
#define NES_EMULATOR_DATAREGISTER_H
#include <cstdint>

#include "PPURegister.h"
#include "internal/flag.hpp"


class DataRegister : PPURegister {
public:
    explicit DataRegister(uint8_t initialValues = 0x0)
        : PPURegister(true, true, initialValues){
    }
};


#endif //NES_EMULATOR_DATAREGISTER_H
