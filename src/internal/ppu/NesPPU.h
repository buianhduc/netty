//
// Created by Đức Bùi on 15/5/2026.
//

#pragma once
#include <cstdint>
#include <span>
#include <utility>
#include <vector>
#include <format>

#include "registers/AddrRegister.h"
#include "registers/ControllerRegister.h"
#include "registers/DataRegister.h"
#include "registers/MaskRegister.h"
#include "registers/OamAddressRegister.h"
#include "registers/OamDataRegister.h"
#include "registers/OamDmaRegister.h"
#include "registers/ScrollRegister.h"
#include "registers/StatusRegister.h"
#include "../rom.h"


class NesPPU {
public:
    std::vector<uint8_t> chr_rom;
    std::vector<uint8_t> chr_ram = std::vector<uint8_t>(8192, 0);
    Mirroring mirroring;
    std::vector<uint8_t> vram = std::vector<uint8_t>(2048, 0);
    std::vector<uint8_t> oam_data = std::vector<uint8_t>(256, 0);
    std::vector<uint8_t> palette_table = std::vector<uint8_t>(32, 0);

    // Registers
    // 0x2000
    ControllerRegister controller_register = ControllerRegister();
    // 0x2001
    MaskRegister mask_register = MaskRegister();
    // 0x2002
    StatusRegister status_register = StatusRegister();
    // 0x2003
    OamAddressRegister oam_address_register = OamAddressRegister();
    // 0x2004
    OamDataRegister oam_data_register = OamDataRegister();
    // 0x2005
    ScrollRegister scroll_register = ScrollRegister();
    // 0x2006
    AddrRegister addr_register = AddrRegister();
    // 0x2007
    DataRegister data_register = DataRegister();
    // 0x4014
    OamDmaRegister oam_dma_register = OamDmaRegister();
    uint8_t internal_data_buf = 0;
    uint8_t io_data_bus = 0;
    bool first_register_write = true;
    uint cycles_ = 0;
    uint16_t scanline = 0;
    std::optional<uint16_t> nmi_interrupt = std::nullopt;

    [[nodiscard]] uint8_t read_ppu_memory(uint16_t addr) const;
    void write_ppu_memory(uint16_t addr, uint8_t data);
    [[nodiscard]] static uint8_t palette_index(uint16_t addr);

public:
    explicit NesPPU(std::vector<uint8_t> chr_rom, Mirroring mirroring): chr_rom(std::move(chr_rom)), mirroring(mirroring) {}

    ;

    ~NesPPU() = default;

    void write_to_controller(uint8_t value);

    void increment_vram_address();

    uint8_t read_status();

    uint8_t read_oam_data();

    uint8_t read_data();

    uint8_t read_open_bus() const;

    uint8_t peek_register(uint16_t address) const;

    uint8_t peek_memory(uint16_t addr) const;

    uint16_t mirror_vram_addr(uint16_t addr) const;

    void write_to_ppu_addr(uint8_t data);

    std::optional<uint8_t> poll_nmi_interrupt();

    void write_to_data(uint8_t data);

    void write_to_mask(uint8_t data);

    void write_to_oam_address(uint8_t data);

    void write_to_oam_data(uint8_t data);

    void write_to_scroll(uint8_t data);

    void write_to_oam_dma(uint8_t page, std::span<const uint8_t, 256> data);

    bool tick(uint64_t i);
};
