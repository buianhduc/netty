#pragma once
#include <cstdint>
#include <vector>

#include "Joypad.h"
#include "rom.h"
#include "ppu/NesPPU.h"

const uint16_t RAM = 0x0;
const uint16_t RAM_MIRROR_END = 0x1FFF;
const uint16_t PPU_REGISTERS = 0x2000;
const uint16_t PPU_REGISTERS_MIRROR_END = 0x3FFF;


class Bus {
    uint64_t cycles_ = 0;
    bool oam_dma_requested_ = false;
    bool frame_complete_ = false;

    public:
    explicit Bus(const ROM &rom);

    Bus() : Bus(ROM()) {}
    ~Bus() = default;
    [[nodiscard]] uint8_t read(uint16_t address, bool readable) const;
    [[nodiscard]] uint16_t read_u16(uint16_t address, bool readable) const;
    [[nodiscard]] uint8_t peek(uint16_t address) const;
    void write(uint16_t address, uint8_t data, bool writeable);
    void write_u16(uint16_t address, uint16_t data, bool writeable);
    void write_prg_rom(uint16_t address, uint8_t data);
    [[nodiscard]] uint8_t read_prg_rom(uint16_t address) const;
    void tick(uint64_t cycles) {
        cycles_ += cycles;
        frame_complete_ = nes_ppu_.tick(cycles * 3) || frame_complete_;
    }

    [[nodiscard]] uint64_t cycles() const { return cycles_; }
    [[nodiscard]] NesPPU& ppu() { return nes_ppu_; }
    [[nodiscard]] const NesPPU& ppu() const { return nes_ppu_; }
    [[nodiscard]] bool take_frame_complete() {
        const bool result = frame_complete_;
        frame_complete_ = false;
        return result;
    }
    [[nodiscard]] Joypad& joypad() { return joypad_; }
    [[nodiscard]] uint16_t take_oam_dma_stall_cycles(uint64_t cpu_cycles);
    std::optional<uint8_t> poll_nmi_status();


private:
        std::vector<uint8_t> cpu_vram_ = std::vector<uint8_t>(2048, 0);
        std::vector<uint8_t> prg_ram_ = std::vector<uint8_t>(0x2000, 0);
        ROM rom;
        mutable NesPPU nes_ppu_;
        mutable Joypad joypad_;
        uint8_t mmc1_shift_register_ = 0x10;
        uint8_t mmc1_control_ = 0x0c;
        uint8_t mmc1_chr_bank0_ = 0;
        uint8_t mmc1_chr_bank1_ = 0;
        uint8_t mmc1_prg_bank_ = 0;

        [[nodiscard]] uint8_t read_mmc1_prg_rom(uint16_t address) const;
        void write_mmc1(uint16_t address, uint8_t data);
        void update_mmc1_chr_banks();
        void update_mmc1_mirroring();
};
