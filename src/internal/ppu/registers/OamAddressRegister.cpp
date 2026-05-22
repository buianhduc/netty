//
// Created by Đức Bùi on 19/5/2026.
//

#include "OamAddressRegister.h"

void OamAddressRegister::update(uint8_t data) {
    value_ = data;
}

void OamAddressRegister::increment() {
    ++value_;
}

uint8_t OamAddressRegister::get() const {
    return value_;
}
