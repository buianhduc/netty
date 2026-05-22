//
// Created by Đức Bùi on 18/5/2026.
//

#include "ControllerRegister.h"

#include <format>
#include <stdexcept>

void ControllerRegister::update(const uint8_t data) {
    this->status = data;
}

uint16_t ControllerRegister::get_name_table_address() const {
    uint8_t base_table_address = this->status & 0b00000011;
    switch (this->status & 0b00000011) {
        case 0x00:
            return 0x2000u;
        case 0x01:
            return 0x2400u;
        case 0x02:
            return 0x2800u;
        case 0x03:
            return 0x2C00u;
        default:
            throw std::runtime_error(
                std::format("Controller Register trying to read invalid base table address :#x",
                                            base_table_address));
    }
}

uint16_t ControllerRegister::get_sprite_pattern_table_address() {
    if (this->is_set(SPRITE_PATTERN_ADDR))
        return 0x1000;
    return 0x0000;
}

uint16_t ControllerRegister::get_background_pattern_address() {
    if (this->is_set(BACKGROUND_PATTERN_ADDR))
        return 0x1000;
    return 0x0000;
}

/**
 *
 * @return 0 if 8x8 pixel, 1 if 8x16 pixels
 */
uint8_t ControllerRegister::get_sprite_size() const {
    return this->status & SPRITE_SIZE;
}

bool ControllerRegister::master_slave_select() const {
    return this->status & MASTER_SLAVE_SELECT;
}

bool ControllerRegister::vblank_nmi_enabled() const {
    return this->status & VBLANK_NMI_ENABLED;
}

uint8_t ControllerRegister::vram_addr_increment() {
    if (this->is_set(VRAM_ADD_INCREMENT)) {
        return 32;
    }
    return 1;
}
