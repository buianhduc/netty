//
// Created by Đức Bùi on 20/5/2026.
//
#include "gtest/gtest.h"

#include <vector>

#include "common/Frame.h"
#include "internal/ppu/NesPPU.h"

TEST(PPU_TEST, TEST_VRAM_WRITES) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);
    ppu.write_to_ppu_addr(0x23);
    ppu.write_to_ppu_addr(0x05);
    ppu.write_to_ppu_addr(0x66);

}

TEST(PPU_TEST, PPUDATAWritesToCurrentVRAMAddressAndIncrementsByOne) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);

    ppu.write_to_ppu_addr(0x20);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x12);
    ppu.write_to_data(0x34);

    EXPECT_EQ(ppu.peek_memory(0x2000), 0x12);
    EXPECT_EQ(ppu.peek_memory(0x2001), 0x34);
}

TEST(PPU_TEST, PPUDATAUsesPPUCTRLIncrementByThirtyTwo) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);

    ppu.write_to_controller(0x04);
    ppu.write_to_ppu_addr(0x20);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x12);
    ppu.write_to_data(0x34);

    EXPECT_EQ(ppu.peek_memory(0x2000), 0x12);
    EXPECT_EQ(ppu.peek_memory(0x2020), 0x34);
}

TEST(PPU_TEST, PPUDATAReadsNametableThroughReadBuffer) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);

    ppu.write_to_ppu_addr(0x21);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x56);
    ppu.write_to_ppu_addr(0x21);
    ppu.write_to_ppu_addr(0x00);

    EXPECT_EQ(ppu.read_data(), 0x00);
    EXPECT_EQ(ppu.read_data(), 0x56);
}

TEST(PPU_TEST, PPUDATAPaletteReadsAreNotBuffered) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);

    ppu.write_to_ppu_addr(0x3f);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x21);
    ppu.write_to_ppu_addr(0x3f);
    ppu.write_to_ppu_addr(0x00);

    EXPECT_EQ(ppu.read_data(), 0x21);
}

TEST(PPU_TEST, PPUDATAPaletteMirrorsUniversalBackgroundEntries) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);

    ppu.write_to_ppu_addr(0x3f);
    ppu.write_to_ppu_addr(0x10);
    ppu.write_to_data(0x2a);

    EXPECT_EQ(ppu.peek_memory(0x3f00), 0x2a);
    EXPECT_EQ(ppu.peek_memory(0x3f10), 0x2a);
}

TEST(PPU_TEST, MirrorsNametableAddressesHorizontally) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);

    ppu.write_to_ppu_addr(0x24);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x44);
    ppu.write_to_ppu_addr(0x2c);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x88);

    EXPECT_EQ(ppu.peek_memory(0x2000), 0x44);
    EXPECT_EQ(ppu.peek_memory(0x2400), 0x44);
    EXPECT_EQ(ppu.peek_memory(0x2800), 0x88);
    EXPECT_EQ(ppu.peek_memory(0x2c00), 0x88);
}

TEST(PPU_TEST, MirrorsNametableAddressesVertically) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Vertical);

    ppu.write_to_ppu_addr(0x28);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x44);
    ppu.write_to_ppu_addr(0x2c);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x88);

    EXPECT_EQ(ppu.peek_memory(0x2000), 0x44);
    EXPECT_EQ(ppu.peek_memory(0x2800), 0x44);
    EXPECT_EQ(ppu.peek_memory(0x2400), 0x88);
    EXPECT_EQ(ppu.peek_memory(0x2c00), 0x88);
}

TEST(PPU_TEST, MirrorsNametableRangeFrom3000To2EFF) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);

    ppu.write_to_ppu_addr(0x30);
    ppu.write_to_ppu_addr(0x00);
    ppu.write_to_data(0x77);

    EXPECT_EQ(ppu.peek_memory(0x2000), 0x77);
    EXPECT_EQ(ppu.peek_memory(0x3000), 0x77);
}

TEST(PPU_TEST, SetsVblankStatusEvenWhenNmiIsDisabled) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);
    ppu.write_to_controller(0x00);

    for (int scanline = 0; scanline < 241; ++scanline) {
        EXPECT_FALSE(ppu.tick(341));
    }

    EXPECT_NE(ppu.read_status() & 0x80, 0);
}

TEST(PPU_TEST, TickCanAdvanceMultipleScanlines) {
    auto ppu = NesPPU(std::vector<uint8_t>(2048, 0), Mirroring::Horizontal);

    EXPECT_FALSE(ppu.tick(341 * 240));
    EXPECT_FALSE(ppu.tick(341));

    EXPECT_NE(ppu.read_status() & 0x80, 0);
}

TEST(FrameRendering, DecodesPatternTableTileBitplanesLeftToRight) {
    std::vector<uint8_t> chr_rom(0x1000, 0);
    chr_rom[0] = 0b10000000;
    chr_rom[8] = 0b01000000;

    const std::vector<Color> palette = {
        ColorHelper::SYSTEM_PALLETE[0x0f],
        ColorHelper::SYSTEM_PALLETE[0x01],
        ColorHelper::SYSTEM_PALLETE[0x02],
        ColorHelper::SYSTEM_PALLETE[0x03],
    };
    const auto frame = render_pattern_table(chr_rom, 0, palette);

    EXPECT_EQ(frame.data[0], ColorHelper::SYSTEM_PALLETE[0x01].red);
    EXPECT_EQ(frame.data[1], ColorHelper::SYSTEM_PALLETE[0x01].green);
    EXPECT_EQ(frame.data[2], ColorHelper::SYSTEM_PALLETE[0x01].blue);

    EXPECT_EQ(frame.data[3], ColorHelper::SYSTEM_PALLETE[0x02].red);
    EXPECT_EQ(frame.data[4], ColorHelper::SYSTEM_PALLETE[0x02].green);
    EXPECT_EQ(frame.data[5], ColorHelper::SYSTEM_PALLETE[0x02].blue);
}

TEST(FrameRendering, RendersBackgroundTileFromNametable) {
    std::vector<uint8_t> chr_rom(0x2000, 0);
    chr_rom[16] = 0b10000000;

    auto ppu = NesPPU(chr_rom, Mirroring::Horizontal);
    ppu.palette_table[0] = 0x0f;
    ppu.palette_table[1] = 0x01;
    ppu.vram[0] = 1;

    Frame frame;
    render(ppu, frame);

    EXPECT_EQ(frame.data[0], ColorHelper::SYSTEM_PALLETE[0x01].red);
    EXPECT_EQ(frame.data[1], ColorHelper::SYSTEM_PALLETE[0x01].green);
    EXPECT_EQ(frame.data[2], ColorHelper::SYSTEM_PALLETE[0x01].blue);
}
