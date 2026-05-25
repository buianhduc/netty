//
// Created by Đức Bùi on 14/5/2026.
//

#pragma once
#include <cstdint>
#include <vector>

enum Mirroring {
    Vertical,
    Horizontal,
    FourScreen
};

class ROM {


    public:
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;
    std::vector<uint8_t> chr_ram;
    bool uses_chr_ram = false;
    uint8_t mapper = 0;
    Mirroring screen_mirroring = Horizontal;
    ROM();
    explicit ROM(const std::vector<uint8_t>& raw);
    ~ROM()=default;
};
