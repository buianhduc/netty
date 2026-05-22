//
// Created by Đức Bùi on 19/5/2026.
//

#include "StatusRegister.h"

uint8_t StatusRegister::read(uint8_t open_bus) {
    const uint8_t result = (status & 0xe0u) | (open_bus & 0x1fu);
    clear_vblank_started();
    return result;
}

uint8_t StatusRegister::peek(uint8_t open_bus) const {
    return (status & 0xe0u) | (open_bus & 0x1fu);
}

void StatusRegister::set_vblank_started(bool value) {
    value ? set(VBLANK_STARTED) : remove(VBLANK_STARTED);
}

void StatusRegister::clear_vblank_started() {
    remove(VBLANK_STARTED);
}

void StatusRegister::set_sprite_zero_hit(bool value) {
    value ? set(SPRITE_ZERO_HIT) : remove(SPRITE_ZERO_HIT);
}

void StatusRegister::clear_sprite_zero_hit() {
    remove(SPRITE_ZERO_HIT);
}

void StatusRegister::set_sprite_overflow() {
    set(SPRITE_OVERFLOW);
}

void StatusRegister::clear_sprite_overflow() {
    remove(SPRITE_OVERFLOW);
}

void StatusRegister::clear_rendering_flags() {
    remove(static_cast<uint8_t>(VBLANK_STARTED | SPRITE_ZERO_HIT | SPRITE_OVERFLOW));
}

bool StatusRegister::is_in_vblank() const {
    return is_set(VBLANK_STARTED);
}
