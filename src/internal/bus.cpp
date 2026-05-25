#include "bus.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <stdexcept>

namespace {
constexpr size_t kPrgBankSize = 0x4000;
constexpr size_t kChr4kBankSize = 0x1000;
constexpr size_t kChr8kBankSize = 0x2000;

size_t clamp_bank(uint8_t bank, size_t bank_count) {
    if (bank_count == 0) {
        return 0;
    }
    return bank % bank_count;
}
}

Bus::Bus(const ROM &rom) : rom(rom), nes_ppu_(rom.chr_rom, rom.screen_mirroring, rom.chr_ram) {
    if (this->rom.mapper == 1) {
        update_mmc1_chr_banks();
        update_mmc1_mirroring();
    }
}

void Bus::write(const uint16_t address, const uint8_t data, bool writeable) {
    if (!writeable) {
        throw std::runtime_error("Attempting to write to non-writeable bus");
    }

    if (std::clamp(address, RAM, RAM_MIRROR_END)==address){
        // Only get the first 11 bits
        auto mirror_down_address = address % 0x0800;
        cpu_vram_[mirror_down_address] = data;
        return;
    }
    if (std::clamp(address, PPU_REGISTERS, PPU_REGISTERS_MIRROR_END) == address) {
        const uint16_t register_address = static_cast<uint16_t>(0x2000u + ((address - 0x2000u) % 8u));
        switch (register_address) {
            case 0x2000:
                nes_ppu_.write_to_controller(data);
                return;
            case 0x2001:
                nes_ppu_.write_to_mask(data);
                return;
            case 0x2003:
                nes_ppu_.write_to_oam_address(data);
                return;
            case 0x2004:
                nes_ppu_.write_to_oam_data(data);
                return;
            case 0x2005:
                nes_ppu_.write_to_scroll(data);
                return;
            case 0x2006:
                nes_ppu_.write_to_ppu_addr(data);
                return;
            case 0x2007:
                nes_ppu_.write_to_data(data);
                return;
        }
    }
    if (address == 0x4014) {
        std::array<uint8_t, 256> page_data{};
        const uint16_t source = static_cast<uint16_t>(data) << 8;
        for (uint16_t offset = 0; offset < page_data.size(); ++offset) {
            page_data[offset] = read(static_cast<uint16_t>(source + offset), true);
        }
        nes_ppu_.write_to_oam_dma(data, page_data);
        oam_dma_requested_ = true;
        return;
    }
    if (address == 0x4016) {
        joypad_.write(data);
        return;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x4000u),
        static_cast<uint16_t>(0x401fu)) == address) {
        // TODO: implement APU and I/O registers.
        return;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x4020u),
        static_cast<uint16_t>(0x5fffu)) == address) {
        // Cartridge expansion area. NROM does not decode it.
        return;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x6000u),
        static_cast<uint16_t>(0x7fffu)) == address) {
        prg_ram_[address - 0x6000u] = data;
        return;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x8000u),
       static_cast<uint16_t>(0xffffu))== address) {
        write_prg_rom(address, data);
        return;
    }
    throw std::invalid_argument("Address out of range trying to write: " + std::to_string(address));
}

void Bus::write_prg_rom(uint16_t address, uint8_t data) {
    if (rom.mapper == 1) {
        write_mmc1(address, data);
    }
    // Mapper 0 has no writable mapper registers, so writes are ignored.
}

uint8_t Bus::read_prg_rom(uint16_t address) const {
    if (rom.prg_rom.empty()) {
        return 0;
    }

    if (rom.mapper == 1) {
        return read_mmc1_prg_rom(address);
    }

    address -= 0x8000;
    if (rom.prg_rom.size() == kPrgBankSize && address >= kPrgBankSize) {
        address %= kPrgBankSize;
    }
    return rom.prg_rom[address % rom.prg_rom.size()];
}

uint8_t Bus::read_mmc1_prg_rom(uint16_t address) const {
    const size_t bank_count = rom.prg_rom.size() / kPrgBankSize;
    if (bank_count == 0) {
        return 0;
    }

    const uint8_t prg_mode = static_cast<uint8_t>((mmc1_control_ >> 2) & 0x03u);
    size_t bank = 0;
    size_t offset = (address - 0x8000u) % kPrgBankSize;

    if (prg_mode <= 1) {
        bank = clamp_bank(static_cast<uint8_t>(mmc1_prg_bank_ & 0x0eu), bank_count);
        offset = address - 0x8000u;
        return rom.prg_rom[(bank * kPrgBankSize + offset) % rom.prg_rom.size()];
    }

    if (address < 0xc000) {
        bank = prg_mode == 2 ? 0 : clamp_bank(mmc1_prg_bank_, bank_count);
    } else {
        bank = prg_mode == 2 ? clamp_bank(mmc1_prg_bank_, bank_count) : bank_count - 1;
    }

    return rom.prg_rom[(bank * kPrgBankSize + offset) % rom.prg_rom.size()];
}

void Bus::write_mmc1(uint16_t address, uint8_t data) {
    if ((data & 0x80u) != 0) {
        mmc1_shift_register_ = 0x10;
        mmc1_control_ |= 0x0c;
        update_mmc1_chr_banks();
        update_mmc1_mirroring();
        return;
    }

    const bool register_complete = (mmc1_shift_register_ & 0x01u) != 0;
    mmc1_shift_register_ >>= 1;
    mmc1_shift_register_ |= static_cast<uint8_t>((data & 0x01u) << 4);

    if (!register_complete) {
        return;
    }

    const uint8_t value = static_cast<uint8_t>(mmc1_shift_register_ & 0x1fu);
    mmc1_shift_register_ = 0x10;

    if (address < 0xa000) {
        mmc1_control_ = value;
        update_mmc1_chr_banks();
        update_mmc1_mirroring();
    } else if (address < 0xc000) {
        mmc1_chr_bank0_ = value;
        update_mmc1_chr_banks();
    } else if (address < 0xe000) {
        mmc1_chr_bank1_ = value;
        update_mmc1_chr_banks();
    } else {
        mmc1_prg_bank_ = value;
    }
}

void Bus::update_mmc1_chr_banks() {
    if (rom.chr_rom.empty()) {
        return;
    }

    nes_ppu_.chr_rom.assign(kChr8kBankSize, 0);
    const bool chr_4k_mode = (mmc1_control_ & 0x10u) != 0;

    if (!chr_4k_mode) {
        const size_t bank_count = std::max<size_t>(1, rom.chr_rom.size() / kChr8kBankSize);
        const size_t bank = clamp_bank(static_cast<uint8_t>(mmc1_chr_bank0_ >> 1), bank_count);
        const size_t source = bank * kChr8kBankSize;
        std::copy_n(rom.chr_rom.begin() + source,
                    std::min(kChr8kBankSize, rom.chr_rom.size() - source),
                    nes_ppu_.chr_rom.begin());
        return;
    }

    const size_t bank_count = std::max<size_t>(1, rom.chr_rom.size() / kChr4kBankSize);
    const size_t bank0 = clamp_bank(mmc1_chr_bank0_, bank_count);
    const size_t bank1 = clamp_bank(mmc1_chr_bank1_, bank_count);
    std::copy_n(rom.chr_rom.begin() + bank0 * kChr4kBankSize,
                std::min(kChr4kBankSize, rom.chr_rom.size() - bank0 * kChr4kBankSize),
                nes_ppu_.chr_rom.begin());
    std::copy_n(rom.chr_rom.begin() + bank1 * kChr4kBankSize,
                std::min(kChr4kBankSize, rom.chr_rom.size() - bank1 * kChr4kBankSize),
                nes_ppu_.chr_rom.begin() + kChr4kBankSize);
}

void Bus::update_mmc1_mirroring() {
    switch (mmc1_control_ & 0x03u) {
        case 2:
            nes_ppu_.mirroring = Vertical;
            break;
        case 3:
            nes_ppu_.mirroring = Horizontal;
            break;
        default:
            nes_ppu_.mirroring = Horizontal;
            break;
    }
}

void Bus::write_u16(const uint16_t address, uint16_t data, bool writeable)
{
    const uint8_t hi = data >> 8;
    const uint8_t lo = data & 0xffu;
    write(address, lo, writeable);
    write(address+1, hi, writeable);
}

std::optional<uint8_t> Bus::poll_nmi_status() {
    return nes_ppu_.poll_nmi_interrupt();
}

uint16_t Bus::take_oam_dma_stall_cycles(uint64_t cpu_cycles) {
    if (!oam_dma_requested_) {
        return 0;
    }

    oam_dma_requested_ = false;
    return (cpu_cycles % 2u == 0u) ? 513 : 514;
}

uint8_t Bus::read(uint16_t address, bool readable) const {
    if (!readable) {
        throw std::runtime_error("Attempting to read from non-readable bus");
    }
    if (std::clamp(address, RAM, RAM_MIRROR_END) == address){
        // Only get the first 11 bits
        auto mirror_down_address = address % 0x0800;
        return cpu_vram_[mirror_down_address];
    }
    if (std::clamp(address, PPU_REGISTERS, PPU_REGISTERS_MIRROR_END) == address) {
        const uint16_t register_address = static_cast<uint16_t>(0x2000u + ((address - 0x2000u) % 8u));
        switch (register_address) {
            case 0x2000:
            case 0x2001:
            case 0x2003:
            case 0x2005:
            case 0x2006:
                return 0;
            case 0x2002:
                return nes_ppu_.read_status();
            case 0x2004:
                return nes_ppu_.read_oam_data();
            case 0x2007:
                return nes_ppu_.read_data();
        }
    }

    if (address == 0x4016) {
        return joypad_.read();
    }
    if (address == 0x4017) {
        return 0;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x4000u),
        static_cast<uint16_t>(0x4015u)) == address) {
        // TODO: implement APU and I/O registers.
        return 0;
    }
    if (address == 0x4014) {
        return 0;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x4020u),
        static_cast<uint16_t>(0x5fffu)) == address) {
        // Cartridge expansion area. NROM does not decode it, so keep a stable open-bus value.
        return 0xff;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x6000u),
        static_cast<uint16_t>(0x7fffu)) == address) {
        return prg_ram_[address - 0x6000u];
    }
    if (std::clamp(address, static_cast<uint16_t>(0x8000u),
        static_cast<uint16_t>(0xffffu)) == address) {
        return read_prg_rom(address);
    }
    throw std::invalid_argument("Address out of range trying to read: " + std::to_string(address));
}

uint8_t Bus::peek(uint16_t address) const {
    if (std::clamp(address, RAM, RAM_MIRROR_END) == address) {
        auto mirror_down_address = address % 0x0800;
        return cpu_vram_[mirror_down_address];
    }
    if (std::clamp(address, PPU_REGISTERS, PPU_REGISTERS_MIRROR_END) == address) {
        return nes_ppu_.peek_register(address);
    }
    if (std::clamp(address, static_cast<uint16_t>(0x4000u),
        static_cast<uint16_t>(0x401fu)) == address) {
        return 0xff;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x4020u),
        static_cast<uint16_t>(0x5fffu)) == address) {
        return 0xff;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x6000u),
        static_cast<uint16_t>(0x7fffu)) == address) {
        return prg_ram_[address - 0x6000u];
    }
    if (std::clamp(address, static_cast<uint16_t>(0x8000u),
        static_cast<uint16_t>(0xffffu)) == address) {
        return read_prg_rom(address);
    }
    throw std::invalid_argument("Address out of range trying to peek: " + std::to_string(address));
}

uint16_t Bus::read_u16(uint16_t address, bool readable) const {
    const uint16_t low = read(address, readable);
    const uint16_t high = read(address + 1, readable);
    return (high << 8 | low);
}
