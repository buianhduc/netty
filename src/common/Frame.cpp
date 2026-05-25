//
// Created by Đức Bùi on 20/5/2026.
//

#include "Frame.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace {
constexpr uint kTileSize = 8;
constexpr uint kTileBytes = 16;
constexpr uint kTilesPerRow = WIDTH / kTileSize;
constexpr uint kRowsPerPatternTable = HEIGHT / kTileSize;
constexpr uint kTilesPerPatternTable = 0x100;
constexpr uint kPatternTableBytes = 0x1000;



uint8_t tile_pixel_value(const std::vector<uint8_t>& chr_rom, size_t tile_offset, uint x, uint y) {
    const uint8_t low_plane = chr_rom[tile_offset + y];
    const uint8_t high_plane = chr_rom[tile_offset + y + 8];
    const uint8_t bit = static_cast<uint8_t>(7u - x);
    const uint8_t low_bit = static_cast<uint8_t>((low_plane >> bit) & 0x01u);
    const uint8_t high_bit = static_cast<uint8_t>((high_plane >> bit) & 0x01u);
    return static_cast<uint8_t>(low_bit | (high_bit << 1));
}

std::array<uint8_t, 4> bg_palette(NesPPU& ppu, uint16_t nametable_base, size_t tile_column, size_t tile_row) {
    const auto attr_table_idx = (tile_row / 4) * 8 + tile_column / 4;
    const auto attr_byte = ppu.peek_memory(static_cast<uint16_t>(nametable_base + 0x03c0 + attr_table_idx));
    const auto palette_idx = [tile_row, tile_column, attr_byte]() {
        const auto to_match = std::make_pair(tile_column % 4 / 2, tile_row % 4 / 2);
        if (to_match == std::make_pair(0,0)) {
            return attr_byte & 0b11;
        }
        if (to_match == std::make_pair(1,0)) {
            return (attr_byte>>2) & 0b11;
        }
        if (to_match == std::make_pair(0,1)) {
            return (attr_byte>>4) & 0b11;
        }
        if (to_match == std::make_pair(1,1)) {
            return (attr_byte>>6) & 0b11;
        }
        throw std::out_of_range("BG palette table index out of range");
    }();
    const auto palette_start = static_cast<uint16_t>(0x3f01 + palette_idx * 4);
    return {
        ppu.peek_memory(0x3f00),
        ppu.peek_memory(palette_start),
        ppu.peek_memory(static_cast<uint16_t>(palette_start + 1)),
        ppu.peek_memory(static_cast<uint16_t>(palette_start + 2)),
    };
}

void render_tile(Frame& frame,
                 const std::vector<uint8_t>& chr_rom,
                 size_t tile_offset,
                 uint dst_x,
                 uint dst_y,
                 const std::vector<Color>& palette) {
    for (uint y = 0; y < kTileSize; ++y) {
        for (uint x = 0; x < kTileSize; ++x) {
            const uint8_t value = tile_pixel_value(chr_rom, tile_offset, x, y);
            frame.set_pixel(static_cast<int>(dst_x + x),
                            static_cast<int>(dst_y + y),
                            palette[value]);
        }
    }
}

struct Rect {
    size_t x1;
    size_t y1;
    size_t x2;
    size_t y2;
};

void render_name_table(NesPPU& ppu,
                       Frame& frame,
                       uint16_t nametable_base,
                       const Rect& view_port,
                       int shift_x,
                       int shift_y) {
    const uint16_t pattern_base = ppu.controller_register.get_background_pattern_address();

    for (uint16_t tile_index = 0; tile_index < 0x03c0; ++tile_index) {
        const auto tile_id = ppu.peek_memory(static_cast<uint16_t>(nametable_base + tile_index));
        const auto tile_column = tile_index % 32;
        const auto tile_row = tile_index / 32;
        const auto palette = bg_palette(ppu, nametable_base, tile_column, tile_row);
        const auto tile_offset = static_cast<uint16_t>(pattern_base + tile_id * kTileBytes);

        for (uint y = 0; y < kTileSize; ++y) {
            const auto low_plane = ppu.peek_memory(static_cast<uint16_t>(tile_offset + y));
            const auto high_plane = ppu.peek_memory(static_cast<uint16_t>(tile_offset + y + 8));
            for (uint x = 0; x < kTileSize; ++x) {
                const auto pixel_x = tile_column * kTileSize + x;
                const auto pixel_y = tile_row * kTileSize + y;
                if (pixel_x < view_port.x1 || pixel_x >= view_port.x2 ||
                    pixel_y < view_port.y1 || pixel_y >= view_port.y2) {
                    continue;
                }

                const uint8_t bit = static_cast<uint8_t>(7u - x);
                const uint8_t value = static_cast<uint8_t>(
                    ((low_plane >> bit) & 0x01u) | (((high_plane >> bit) & 0x01u) << 1));
                const auto color = ColorHelper::SYSTEM_PALLETE[palette[value] & 0x3fu];
                frame.set_pixel(static_cast<int>(pixel_x) + shift_x,
                                static_cast<int>(pixel_y) + shift_y,
                                color);
            }
        }
    }
}
}

Frame render_pattern_table(const std::vector<uint8_t>& chr_rom, uint bank,
                           const std::vector<Color>& palette) {
    if (bank > 1) {
        throw std::out_of_range("CHR pattern table bank must be 0 or 1");
    }

    const size_t bank_offset = static_cast<size_t>(bank) * kPatternTableBytes;
    if (chr_rom.size() < bank_offset + kPatternTableBytes) {
        throw std::out_of_range("CHR ROM is too small for requested pattern table bank");
    }

    auto frame = Frame();
    for (uint tile_index = 0; tile_index < kTilesPerPatternTable; ++tile_index) {
        const uint tile_x = tile_index % kTilesPerRow;
        const uint tile_y = tile_index / kTilesPerRow;
        if (tile_y >= kRowsPerPatternTable) {
            break;
        }

        render_tile(frame,
                    chr_rom,
                    bank_offset + static_cast<size_t>(tile_index) * kTileBytes,
                    tile_x * kTileSize,
                    tile_y * kTileSize,
                    palette);
    }
    return frame;
}

Frame show_title(std::vector<uint8_t> chr_rom, uint bank, uint tile_n) {
    if (bank > 1) {
        throw std::out_of_range("CHR pattern table bank must be 0 or 1");
    }

    const size_t tile_offset = static_cast<size_t>(bank) * kPatternTableBytes +
                               static_cast<size_t>(tile_n) * kTileBytes;
    if (chr_rom.size() < tile_offset + kTileBytes) {
        throw std::out_of_range("CHR ROM is too small for requested tile");
    }

    auto frame = Frame();
    // render_tile(frame, chr_rom, tile_offset, 0, 0, {0x0f, 0x30, 0x21, 0x11});
    return frame;
}


std::array<uint8_t, 4> get_sprite_palette(const NesPPU & nes_ppu, int
    pallette_idx) {
    const auto start = static_cast<uint16_t>(0x3f11 + pallette_idx * 4);
    return {
        0,
        nes_ppu.peek_memory(start),
        nes_ppu.peek_memory(static_cast<uint16_t>(start + 1)),
        nes_ppu.peek_memory(static_cast<uint16_t>(start + 2))
    };
};

void render(NesPPU &nes_ppu, Frame &frame) {
    const uint16_t nametable_base = nes_ppu.controller_register.get_name_table_address();
    const size_t scroll_x = nes_ppu.scroll_register.x_scroll();
    const size_t scroll_y = nes_ppu.scroll_register.y_scroll();

    uint16_t main_nametable = 0x2000;
    uint16_t second_nametable = 0x2400;
    const auto mirroring = nes_ppu.mirroring;
    if ((mirroring == Vertical && (nametable_base == 0x2000 || nametable_base == 0x2800)) ||
        (mirroring == Horizontal && (nametable_base == 0x2000 || nametable_base == 0x2400))) {
        main_nametable = 0x2000;
        second_nametable = 0x2400;
    } else if ((mirroring == Vertical && (nametable_base == 0x2400 || nametable_base == 0x2c00)) ||
               (mirroring == Horizontal && (nametable_base == 0x2800 || nametable_base == 0x2c00))) {
        main_nametable = 0x2400;
        second_nametable = 0x2000;
    } else {
        throw std::runtime_error("Unsupported mirroring type or nametable base");
    }

    render_name_table(
        nes_ppu,
        frame,
        main_nametable,
        Rect{scroll_x, scroll_y, 256, 240},
        -static_cast<int>(scroll_x),
        -static_cast<int>(scroll_y));

    if (scroll_x > 0) {
        render_name_table(
            nes_ppu,
            frame,
            second_nametable,
            Rect{0, 0, scroll_x, 240},
            static_cast<int>(256 - scroll_x),
            0);
    } else if (scroll_y > 0) {
        render_name_table(
            nes_ppu,
            frame,
            second_nametable,
            Rect{0, 0, 256, scroll_y},
            0,
            static_cast<int>(240 - scroll_y));
    }

    // Render Sprite
    for (int i = static_cast<int>(nes_ppu.oam_data.size()) - 4; i >= 0; i -= 4) {
        const auto tile_idx = nes_ppu.oam_data[i + 1];
        const auto tile_x = nes_ppu.oam_data[i + 3];
        const auto tile_y = nes_ppu.oam_data[i];

        const auto attributes = nes_ppu.oam_data[i + 2];
        const bool flip_vertical = ((attributes >> 7) & 1) == 1;
        const bool flip_horizontal = ((attributes >> 6) & 1) == 1;

        const auto palette_idx = attributes & 0b11;
        const auto sprite_palette = get_sprite_palette(nes_ppu, palette_idx);
        const auto bank = nes_ppu.controller_register.get_sprite_pattern_table_address();
        const auto tile_offset = static_cast<uint16_t>(bank + tile_idx * kTileBytes);
        for (int y = 0; y < static_cast<int>(kTileSize); ++y) {
            auto low_plane = nes_ppu.peek_memory(static_cast<uint16_t>(tile_offset + y + 8));
            auto high_plane = nes_ppu.peek_memory(static_cast<uint16_t>(tile_offset + y));
            for (int x = 0; x < static_cast<int>(kTileSize); ++x) {
                const auto value = (1 & low_plane) << 1 | (1 & high_plane);
                high_plane >>= 1;
                low_plane >>= 1;
                if (value == 0) {
                    continue;
                }

                const int pixel_x = tile_x + (!flip_horizontal ? 7 - x : x);
                const int pixel_y = tile_y + (flip_vertical ? 7 - y : y);
                frame.set_pixel(pixel_x,
                                pixel_y,
                                ColorHelper::SYSTEM_PALLETE[sprite_palette[value]]);
            }
        }
    }
}
