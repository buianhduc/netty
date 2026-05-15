//
// Created by Đức Bùi on 15/5/2026.
//

#pragma once
#include <cstdint>
#include <utility>
#include <vector>

#include "rom.h"


class NesPPU {
    std::vector<uint8_t> chr_rom;
    Mirroring mirroring;
    std::vector<uint8_t> vram = std::vector<uint8_t>(2048, 0);
    std::vector<uint8_t> oam_data = std::vector<uint8_t>(256, 0);
    std::vector<uint8_t> palette_table = std::vector<uint8_t>(32, 0);
    public:
    explicit NesPPU(std::vector<uint8_t> chr_rom, Mirroring mirroring): chr_rom(std::move(chr_rom)), mirroring(mirroring) {};
    ~NesPPU() = default;
};

