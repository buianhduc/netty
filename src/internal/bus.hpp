#pragma once
#include <cstdint>
#include <vector>

#include "rom.h"

const uint16_t RAM = 0x0;
const uint16_t RAM_MIRROR_END = 0x1FFF;
const uint16_t PPU_REGISTERS = 0x2000;
const uint16_t PPU_REGISTERS_MIRROR_END = 0x3FFF;


class Bus {
    public:
    explicit Bus(const ROM &rom) : rom(rom){}
    Bus() = default;
    ~Bus() = default;
    [[nodiscard]] uint8_t read(uint16_t address) const;
    [[nodiscard]] uint16_t read_u16(uint16_t address) const;
    void write(uint16_t address, uint8_t data);
    void write_u16(uint16_t address, uint16_t data);

    [[nodiscard]] uint8_t read_prg_rom(uint16_t address) const {
        address -= 0x8000;
        if (rom.prg_rom.size() == 0x4000 && address >= 0x4000) {
            address %= 0x4000;
        }
        return rom.prg_rom[address];
    }
        
    private:
        std::vector<uint8_t> cpu_vram_ = std::vector<uint8_t>(2048, 0);
        ROM rom;
};
