//
// Created by Đức Bùi on 18/5/2026.
//

#pragma once
#include <cstdint>

#include "PPURegister.h"
#include "internal/flag.hpp"

/**
*
    7  bit  0
    ---- ----
    VPHB SINN
    |||| ||||
    |||| ||++- Base nametable address
    |||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
    |||| |+--- VRAM address increment per CPU read/write of PPUDATA
    |||| |     (0: add 1, going across; 1: add 32, going down)
    |||| +---- Sprite pattern table address for 8x8 sprites
    ||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
    |||+------ Background pattern table address (0: $0000; 1: $1000)
    ||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM#Byte 1)
    |+-------- PPU master/slave select
    |          (0: read backdrop from EXT pins; 1: output color on EXT pins)
    +--------- Vblank NMI enable (0: off, 1: on)

 */

enum ControllerRegisterValue : uint8_t {
    NAMETABLE1 = 1 << 0,
    NAMETABLE2 = 1 << 1,
    VRAM_ADD_INCREMENT = 1 << 2,
    SPRITE_PATTERN_ADDR = 1 << 3,
    BACKGROUND_PATTERN_ADDR = 1 << 4,
    SPRITE_SIZE = 1 << 5,
    MASTER_SLAVE_SELECT = 1 << 6,
    VBLANK_NMI_ENABLED = 1 << 7,

};
class ControllerRegister : PPURegister {
    public:
    explicit ControllerRegister(const uint8_t init_value = 0x0) : PPURegister(false,
                                                                              true,
                                                                              init_value) {}
    ~ControllerRegister() = default;

    uint8_t vram_addr_increment();
    void update(uint8_t data);
    uint16_t get_name_table_address() const;
    uint16_t get_sprite_pattern_table_address();
    uint16_t get_background_pattern_address();
    uint8_t get_sprite_size() const;
    bool master_slave_select() const;
    bool vblank_nmi_enabled() const;

};


