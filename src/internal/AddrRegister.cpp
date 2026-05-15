//
// Created by Đức Bùi on 15/5/2026.
//

#include "AddrRegister.h"

uint16_t AddrRegister::get() const {
    return (static_cast<uint16_t>(value.first) << 8) | value.second;
}

void AddrRegister::increment(uint8_t inc) {
    auto lo = value.first;
    auto hi = value.second;
    value.first = lo + inc;
    if (lo > hi) {
        value.first += 1;
    }
    if (get() > 0x3fff) {
        set_value(get() & 0b11111111111111); //mirror down addr above 0x3fff
    }
}

void AddrRegister::reset_latch() {
    hi_ptr = true;
}

void AddrRegister::update(uint8_t value) {
    if (hi_ptr) {
        this->value.first = value;
    } else {
        this->value.second = value;
    }
    if (get() > 0x3fff) {
        set_value(get() & 0b11111111111111); //mirror down addr above 0x3fff
    }
    hi_ptr = !hi_ptr;
}

void AddrRegister::set_value(uint16_t data) {
    value.first = data >> 8;
    value.second = data & 0xff;
}
