//
// Created by Đức Bùi on 14/5/2026.
//

#include "rom.h"

#include <span>

const std::vector<uint8_t> NES_TAG = std::vector<uint8_t>({0x4E, 0x45, 0x53,
    0x1A});
#define PRG_ROM_PAGE_SIZE (16*1024)
#define CHR_ROM_PAGE_SIZE (8*1024)

ROM::ROM() : prg_rom(0x8000, 0), chr_ram(CHR_ROM_PAGE_SIZE, 0), uses_chr_ram(true) {}

ROM::ROM(const std::vector<uint8_t>& raw) {
    // Read the first 16 bytes
    if (memcmp(std::span(raw.begin(), raw.begin()+4).data(), NES_TAG.data(),
        4) != 0) {
        throw std::invalid_argument("This ROM might not be NES ROM");
    }
    this->mapper = (raw[7] & 0b11110000) | (raw[6] >> 4);
    uint8_t ines_ver = (raw[7] >> 2) & 0b11;
    if (ines_ver != 0) {
        throw std::invalid_argument("NES 2.0 format is not supported");
    }

    bool four_screen = (raw[6] & 0b1000) != 0;
    bool vertical_mirroring = (raw[6] & 0b1) != 0;

    this->screen_mirroring = [four_screen, vertical_mirroring]() {
        if (four_screen) {
            return FourScreen;
        }
        if (!four_screen and vertical_mirroring) {
            return Vertical;
        }
        return Horizontal;
    }();

    // Read numbers of 16kB ROM banks (PRG ROM)
    const auto prg_rom_size = raw[4] * PRG_ROM_PAGE_SIZE;
    const auto chr_rom_size = raw[5] * CHR_ROM_PAGE_SIZE;

    const auto skip_trainer = (raw[6] & 0b100) != 0;

    const auto prg_rom_start = 16 + (skip_trainer ? 512 : 0);
    const auto chr_rom_start = prg_rom_size + prg_rom_start;
    this->prg_rom = std::vector(raw.begin() + prg_rom_start,
        raw.begin() + prg_rom_start +
        prg_rom_size);
    if (chr_rom_size == 0) {
        this->uses_chr_ram = true;
        this->chr_ram = std::vector<uint8_t>(CHR_ROM_PAGE_SIZE, 0);
        this->chr_rom.clear();
    } else {
        this->uses_chr_ram = false;
        this->chr_ram.clear();
        this->chr_rom = std::vector(raw.begin() + chr_rom_start, raw.begin() +
            chr_rom_start +
            chr_rom_size);
    }
}
