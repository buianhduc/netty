//
// Created by Đức Bùi on 19/5/2026.
//

#include "OamDmaRegister.h"

void OamDmaRegister::update(uint8_t data) {
    page_ = data;
}

uint16_t OamDmaRegister::source_address() const {
    return static_cast<uint16_t>(page_) << 8;
}
