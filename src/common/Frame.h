//
// Created by Đức Bùi on 20/5/2026.
//

#ifndef NES_EMULATOR_FRAME_H
#define NES_EMULATOR_FRAME_H
#include <array>
#include <cstdint>
#include <vector>

#include "Color.h"
#include "internal/ppu/NesPPU.h"

#define WIDTH 256
#define HEIGHT 240

class Frame {
public:
    std::vector<uint8_t> data = std::vector<uint8_t>((WIDTH*HEIGHT*3), 0);

    Frame() = default;
    ~Frame() = default;

    void set_pixel(int x, int y, Color color) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
            return;
        }
        auto base = y * 3 * WIDTH + x*3;
        if (base + 2 < data.size()) {
            data[base] = color.red;
            data[base+1] = color.green;
            data[base+2] = color.blue;
        }
    }

};

Frame show_title(std::vector<uint8_t> chr_rom, uint bank, uint tile_n);
Frame render_pattern_table(const std::vector<uint8_t>& chr_rom, uint bank,
                           const std::vector<Color>& palette);
void render(NesPPU& nes_ppu, Frame& frame);

#endif //NES_EMULATOR_FRAME_H
